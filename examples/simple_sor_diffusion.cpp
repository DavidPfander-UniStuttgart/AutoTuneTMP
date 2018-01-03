#include <iostream>
#include <vector>
#include <cmath>
#include <Vc/Vc>
#include <chrono>
//#include "tga.h"

using Vc::double_v;

const int dimX = 512;
const int dimY = 512;
const double eps = 1e-3;
const double hx = 2.0*M_PI/dimX;
const double hy = 2.0*M_PI/dimY;

#define OMEGA 1.955
#define INNER_LOOP 10
#define ITER_MIN 256

const double_v hxx = 1.0/(hx*hx);
const double_v hyy = 1.0/(hy*hy);
const double_v pre = OMEGA * hx*hx*hy*hy/(2.0*(hx*hx + hy*hy));

void InitGrid(std::vector<double> &rhs) {
  for (int y = 0; y < dimY; ++y ) {
    for (int x = 0; x < dimX; ++x) {
      int i = x + (y+1)*(dimX + 2) + 1;
      rhs[i] = -.5*sin(hx*x)*sin(hy*y);
    }
  }
}

// 9 * VEC_SIZE FLOPs
inline double_v DiffusionKernel (const double_v &di, const double_v &dt, const double_v &db,
                                 const double_v &dl, const double_v &dr) {
  return ((dl + dr - 2.0*di)*hxx + (dt + db - 2.0*di)*hyy);
}

// 3 + 9 = 12 * VEC_SIZE FLOPs
template<int line, int R>
inline void SORCycleKernel (double *red, const double *black, const double *rhs) {
  double_v di = double_v( red,             Vc::flags::element_aligned);
  double_v dt = double_v(&black[2*line],   Vc::flags::element_aligned);
  double_v db = double_v( black,           Vc::flags::element_aligned);
  double_v dl = double_v(&black[line+R-1], Vc::flags::element_aligned);
  double_v dr = double_v(&black[line+R],   Vc::flags::element_aligned);
  double_v  r = double_v( rhs,             Vc::flags::element_aligned);
  r -= DiffusionKernel(di, dt, db, dl, dr);
  di -= pre * r;
  di.memstore(red, Vc::flags::element_aligned);
}
// 2 + 9 = 11 * VEC_SIZE  + VEC_SIZE - 1  FLOPs
template<int line, int R>
inline double SORResidualKernel (const double *red, const double *black, const double *rhs) {
  double_v di = double_v( red,             Vc::flags::element_aligned);
  double_v dt = double_v(&black[2*line],   Vc::flags::element_aligned);
  double_v db = double_v( black,           Vc::flags::element_aligned);
  double_v dl = double_v(&black[line+R-1], Vc::flags::element_aligned);
  double_v dr = double_v(&black[line+R],   Vc::flags::element_aligned);
  double_v  r = double_v( rhs,             Vc::flags::element_aligned);
  r -= DiffusionKernel(di, dt, db, dl, dr);
  return Vc::reduce(r*r);
}

template<int line, int R>
void RBSORUpdate (std::vector<double> &grid) {
  for (int y = 1; y <= dimY; ++y) {
    if (y%2 == R) {
      grid[y*line + line - 1] = grid[y*line];
    } else {
      grid[y*line] = grid[y*line + line - 1];
    }
  }
  for (int x = 0; x < line; ++x) {
    grid[x] = grid[dimY*line + x];
    grid[(dimY + 1)*line + x] = grid[line + x];
  }
}

constexpr int RED = 1;
constexpr int BLACK = 0;

// dimX*dimY*12 * VEC_SIZE  FLOPs
double RBSORResidual(const std::vector<double> &grid_r, const std::vector<double> &grid_b,
                     const std::vector<double> &rhs_r, const std::vector<double> &rhs_b) {
  double res = 0.0;
#pragma omp parallel for reduction(+:res)
  for (int y = 1; y <= dimY; ++y ) {
    for (int x = 0; x < dimX/2; x += double_v::size()) {
      int i = y*(dimX/2 + 1) + x;
      if (y%2 == RED) {
        res += SORResidualKernel<dimX/2 + 1,   RED>(&grid_r[i], &grid_b[i - dimX/2 - 1], &rhs_r[i]);
        ++i;
        res += SORResidualKernel<dimX/2 + 1, BLACK>(&grid_b[i], &grid_r[i - dimX/2 - 1], &rhs_b[i]);
      } else {
        res += SORResidualKernel<dimX/2 + 1,   RED>(&grid_b[i], &grid_r[i - dimX/2 - 1], &rhs_b[i]);
        ++i;
        res += SORResidualKernel<dimX/2 + 1, BLACK>(&grid_r[i], &grid_b[i - dimX/2 - 1], &rhs_r[i]);
      }
    }
  }
  return res;
}

// dimX*dimY*12 * VEC_SIZE  FLOPs
void RBSORCycle(std::vector<double> &grid_r, std::vector<double> &grid_b,
                const std::vector<double> &rhs_r, const std::vector<double> &rhs_b) {
#pragma omp parallel for
  for (int y = 1; y <= dimY; ++y ) {
    for (int x = 0; x < dimX/2; x += double_v::size()) {
      int i = y*(dimX/2 + 1) + x;
      if (y%2 == RED) {
        SORCycleKernel<dimX/2 + 1,   RED>(&grid_r[i], &grid_b[i - dimX/2 - 1], &rhs_r[i]);
      } else {
        ++i;
        SORCycleKernel<dimX/2 + 1, BLACK>(&grid_r[i], &grid_b[i - dimX/2 - 1], &rhs_r[i]);
      }
    }
  }
  RBSORUpdate<dimX/2 + 1, RED>(grid_r);
#pragma omp parallel for
  for (int y = 1; y <= dimY; ++y ) {
    for (int x = 0; x < dimX/2; x += double_v::size()) {
      int i = y*(dimX/2 + 1) + x;
      if (y%2 == RED) {
        ++i;
        SORCycleKernel<dimX/2 + 1, BLACK>(&grid_b[i], &grid_r[i - dimX/2 - 1], &rhs_b[i]);
      } else {
        SORCycleKernel<dimX/2 + 1,   RED>(&grid_b[i], &grid_r[i - dimX/2 - 1], &rhs_b[i]);
      }
    }
  }
  RBSORUpdate<dimX/2 + 1, BLACK>(grid_b);
}

size_t SORFlops (size_t iter) {
  return iter*dimX*dimY*12*double_v::size() + (iter/INNER_LOOP)*dimX*dimY*12*double_v::size();
}

// iter*(dimX*dimY*12)* VEC_SIZE  + iter/INNER_LOOP*(dimX*dimY*12) * VEC_SIZE 
size_t SORSolve(std::vector<double> &grid, const std::vector<double> &rhs, const double &eps) {
  const double eps_limit = eps*eps*dimX*dimY;
  size_t iter = 0;
  size_t iter_top = 0;
  double res;
  double res_old = -1.0;
  double res_top = -1.0;
  double rate = 0.0;
  
  res = 0.0;
  std::vector<double> grid_r((dimX+2)*(dimY+2)/2, 0.0);
  std::vector<double> grid_b((dimX+2)*(dimY+2)/2, 0.0);
  
  std::vector<double> rhs_r((dimX+2)*(dimY+2)/2);
  std::vector<double> rhs_b((dimX+2)*(dimY+2)/2);
  for (int y = 1; y <= dimY; ++y ) {
    for (int x = 0; x < dimX/2; ++x) {
      int i = x + y*(dimX/2 + 1);
      if (y%2 == RED) {
        rhs_r[i] = rhs[2*i + 1];
        rhs_b[i+1] = rhs[2*i + 2];
      } else {
        rhs_b[i] = rhs[2*i + 1];
        rhs_r[i+1] = rhs[2*i + 2];
      }
    }
  }
  
  auto start = std::chrono::high_resolution_clock::now();
  while ((res = RBSORResidual(grid_r, grid_b, rhs_r, rhs_b)) > eps_limit) {
    // FLOPs not counted for break conditions
    if (res > res_top) {
      res_top = res;
      iter_top = iter;
    }
    if (iter > ITER_MIN) {
      if (res_top <= res + eps) break;
      if (abs(res_old - res) < eps) break;
    }
    res_old = res;
    
    for (int i = 0; i < INNER_LOOP; ++i) {
      ++iter;
      RBSORCycle(grid_r, grid_b, rhs_r, rhs_b);
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;
  
  for (int y = 1; y <= dimY; ++y ) {
    for (int x = 0; x < dimX/2; ++x) {
      int i = x + y*(dimX/2 + 1);
      if (y%2 == RED) {
        grid[2*i + 1] = grid_r[i];
        grid[2*i + 2] = grid_b[i+1];
      } else {
        grid[2*i + 1] = grid_b[i];
        grid[2*i + 2] = grid_r[i+1];
      }
    }
  }
  std::cout << "Convergence rate: " << pow(res_top/res, 0.5/(iter - iter_top)) - 1.0;
  std::cout << " after " << iter_top << " iterations" << std::endl;
  std::cout << "Solved in " << iter << " iterations" << std::endl;
  std::cout << "Residual: " << sqrt(res/(dimX*dimY)) << std::endl << std::endl;
  std::cout << "Finished in " << diff.count() << "s" << std::endl;
  std::cout << "Executed " << SORFlops(iter) << " FLOPs" << std::endl;
  std::cout << "Achieved " << double(SORFlops(iter))*1e-9/diff.count() << " GFLOP/s" << std::endl;
  return iter;
}

int main (void) {
  std::vector<double> grid((dimX+2)*(dimY+2), 0.0);
  std::vector<double> rhs ((dimX+2)*(dimY+2), 0.0);
  
  InitGrid(rhs);
  SORSolve(grid, rhs, eps);
  
  /*RGB_t img[dimX*dimY];
  double gmin, gmax;
  gmin = gmax = grid[0];
  for (int i = 0; i < dimX*dimY; ++i) {
    if (gmin > grid[i]) gmin = grid[i];
    if (gmax < grid[i]) gmax = grid[i];
  }
   for (int y = 0; y < dimY; ++y ) {
    for (int x = 0; x < dimX; ++x) {
      int i = x + (y+1)*(dimX + 2) + 1;
      byte val = (byte)(255 * (grid[i] - gmin)/(gmax-gmin));
      img[x + y*dimX] = ColorRGB(val, val, val);
    }
  }
  write_tga("SOR_Diffusion.tga", img, dimX, dimY);*/
  
  return 0;
}
