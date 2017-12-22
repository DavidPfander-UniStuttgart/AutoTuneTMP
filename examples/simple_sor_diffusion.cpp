#include <iostream>
#include <vector>
#include <cmath>
#include "tga.h"

const int dimX = 256;
const int dimY = 256;
const double eps = 1e-3;
const double hx = 2.0*M_PI/dimX;
const double hy = 2.0*M_PI/dimY;

#define OMEGA 1.9
#define VEC_SIZE 4
#define INNER_LOOP 10

#define VEC_D(VAR) double VAR [VEC_SIZE]
#define LOOP_VEC(IT) for (int IT = 0; IT < VEC_SIZE; ++IT)

void InitGrid(std::vector<double> &rhs) {
  for (int y = 0; y < dimY; ++y ) {
    for (int x = 0; x < dimX; ++x) {
      int i = x + (y+1)*(dimX + 2) + 1;
      rhs[i] = -.5*sin(hx*x)*sin(hy*y);
    }
  }
}

void SORBoundary(std::vector<double> &grid) {
  for (int x = 1; x <= dimX; x += VEC_SIZE) {
    VEC_D(v);
    LOOP_VEC(j) v[j] = grid[x + j + dimX + 2];
    LOOP_VEC(j) grid[(dimX + 2)*(dimY + 1) + x + j] = v[j];
    LOOP_VEC(j) v[j] = grid[(dimX + 2)*dimY + x + j];
    LOOP_VEC(j) grid[x + j] = v[j];
  }
  for (int y = 1; y <= dimY; y += VEC_SIZE) {
    VEC_D(v);
    LOOP_VEC(j) v[j] = grid[(dimX + 2)*(y + j) + 1];
    LOOP_VEC(j) grid[(dimX + 2)*(y + j) + dimX + 1] = v[j];
    LOOP_VEC(j) v[j] = grid[(dimX + 2)*(y + j) + dimX];
    LOOP_VEC(j) grid[(dimX + 2)*(y + j)] = v[j];
  }
}

void SORCycle(std::vector<double> &grid, const std::vector<double> &rhs) {
  static const double h = hx*hx*hy*hy/(2.0*(hx*hx + hy*hy));
  static const double hxx = 1.0/(hx*hx);
  static const double hyy = 1.0/(hy*hy);
  for (int y = 1; y <= dimY; ++y ) {
    for (int x = 1; x <= dimX; x += 2*VEC_SIZE) {
      for (int k = 0; k < 2; ++k) {
        int i = x + k + y*(dimX + 2);
        VEC_D(di);
        LOOP_VEC(j) di[j] = grid[i + 2*j];
        VEC_D(du);
        LOOP_VEC(j) du[j] = grid[i + 2*j + dimX + 2];
        VEC_D(db);
        LOOP_VEC(j) db[j] = grid[i + 2*j - dimX - 2];
        VEC_D(dl);
        LOOP_VEC(j) dl[j] = grid[i + 2*j - 1];
        VEC_D(dr);
        LOOP_VEC(j) dr[j] = grid[i + 2*j + 1];
        VEC_D(r);
        LOOP_VEC(j) r[j] = rhs[i + 2*j];
        
        LOOP_VEC(j) r[j] -= (dl[j] + dr[j] - 2.0*di[j])*hxx + (du[j] + db[j] - 2.0*di[j])*hyy;
        LOOP_VEC(j) di[j] -= OMEGA * h * r[j];
        
        LOOP_VEC(j) grid[i + 2*j] = di[j];
      }
    }
  }
}
double SORResidual(std::vector<double> &grid, const std::vector<double> &rhs) {
  static const double h = hx*hx*hy*hy/(2.0*(hx*hx + hy*hy));
  static const double hxx = 1.0/(hx*hx);
  static const double hyy = 1.0/(hy*hy);
  double res = 0.0;
  for (int y = 1; y <= dimY; ++y ) {
    for (int x = 1; x <= dimX; x += VEC_SIZE) {
      int i = x + y*(dimX + 2);
      VEC_D(di);
      LOOP_VEC(j) di[j] = grid[i + j];
      VEC_D(du);
      LOOP_VEC(j) du[j] = grid[i + j + dimX + 2];
      VEC_D(db);
      LOOP_VEC(j) db[j] = grid[i + j - dimX - 2];
      VEC_D(dl);
      LOOP_VEC(j) dl[j] = grid[i + j - 1];
      VEC_D(dr);
      LOOP_VEC(j) dr[j] = grid[i + j + 1];
      VEC_D(r);
      LOOP_VEC(j) r[j] = rhs[i + j];
      
      LOOP_VEC(j) r[j] -= (dl[j] + dr[j] - 2.0*di[j])*hxx + (du[j] + db[j] - 2.0*di[j])*hyy;
      LOOP_VEC(j) res += r[j]*r[j];
    }
  }
  return res;
}

size_t SORSolve(std::vector<double> &grid, const std::vector<double> &rhs, 
                double &res, const double &eps) {
  const double eps_limit = eps*eps*dimX*dimY;
  size_t iter = 0;
  double res_old = -1.0;
  
  res = 0.0;
  for (auto &cell : grid) cell = 0.0;
  
  while ((res = SORResidual(grid, rhs)) > eps_limit) {
    //if ((iter % 100) == 0) std::cout << iter << " " << res << std::endl;
    if (iter < 10 + INNER_LOOP && res > res_old) res_old = res;
    if (iter >= 10 + INNER_LOOP && res >= res_old) break;
    res_old = res;
    
    for (int i = 0; i < INNER_LOOP; ++i) {
      ++iter;
      SORCycle(grid, rhs);
      SORBoundary(grid);
    }
  }
  
  return iter;
}

int main (void) {
  std::vector<double> grid((dimX+2)*(dimY+2), 0.0);
  std::vector<double> rhs ((dimX+2)*(dimY+2), 0.0);
  
  InitGrid(rhs);
  double res;
  size_t iter = SORSolve(grid, rhs, res, eps);
  
  std::cout << "Solved in " << iter << " iterations" << std::endl;
  std::cout << "Residual: " << sqrt(res/(dimX*dimY)) << std::endl;
  
  RGB_t img[dimX*dimY];
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
  write_tga("SOR_Diffusion.tga", img, dimX, dimY);
  
  return 0;
}
