#include <Vc/Vc>
#include <chrono>
#include <iostream>

#include "parameters.hpp"
#include "../simple_sor_diffusion.hpp"

using Vc::double_v;
#define VcEA Vc::flags::element_aligned

const double_v hxx = 1.0/(hx*hx);
const double_v hyy = 1.0/(hy*hy);
const double_v pre = OMEGA * hx*hx*hy*hy/(2.0*(hx*hx + hy*hy));

// 9 * VEC_SIZE FLOPs
inline double_v DiffusionKernel (const double_v &di, const double_v &dt, const double_v &db,
                                 const double_v &dl, const double_v &dr) {
  return ((dl + dr - 2.0*di)*hxx + (dt + db - 2.0*di)*hyy);
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
void RBSORUpdate (double *grid) {
#pragma omp parallel for num_threads(NUMTHREADS)
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
double RBSORResidual(const double *grid_r, const double *grid_b,
                     const double *rhs_r,  const double *rhs_b) {
  double res = 0.0;
#pragma omp parallel for reduction(+:res) num_threads(NUMTHREADS)
  for (int y = 1; y <= dimY; y += 1) {
    for (int x = 0; x < dimX/2; x += double_v::size()) {
      int i = (y)*(dimX/2 + 1) + x;
      if ((y)%2 == RED) {
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

// dimX*dimY*12  FLOPs
void RBSORCycle(double *grid_r, double *grid_b, const double *rhs_r, const double *rhs_b) {
#pragma omp parallel for num_threads(NUMTHREADS)
  for (int y = 0; y < dimY; y += 2*BLOCKSIZEY) {
    for (int x = 0; x < dimX/2; x += BLOCKSIZEX*double_v::size()) {
      int idx = y*dimX_half + x;
      std::array<std::array<double_v, BLOCKSIZEX>, 2*BLOCKSIZEY> red;
      std::array<std::array<double_v, BLOCKSIZEX>, 2*BLOCKSIZEY> rhs;
      std::array<std::array<double_v, BLOCKSIZEX>, 2*BLOCKSIZEY + 1> blk_l;
      std::array<std::array<double_v, BLOCKSIZEX>, 2*BLOCKSIZEY + 1> blk_r;
      for (int kx = 0; kx < BLOCKSIZEX; ++kx) {
        blk_l[0           ][kx] = double_v(&grid_b[idx + kx*double_v::size()                             ], VcEA);
        blk_r[2*BLOCKSIZEY][kx] = double_v(&grid_b[idx + (2*BLOCKSIZEY+1)*dimX_half + kx*double_v::size() + 1], VcEA);
      }
      for (int ky = 0; ky < 2*BLOCKSIZEY; ky+=2) {
        for (int kx = 0; kx < BLOCKSIZEX; ++kx) {
          red[ky  ][kx]   = double_v(&grid_r[idx + (ky+1)*dimX_half + kx*double_v::size()    ], VcEA);
          rhs[ky  ][kx]   = double_v( &rhs_r[idx + (ky+1)*dimX_half + kx*double_v::size()    ], VcEA);
          blk_l[ky+1][kx] = double_v(&grid_b[idx + (ky+1)*dimX_half + kx*double_v::size()    ], VcEA);
          blk_r[ky  ][kx] = double_v(&grid_b[idx + (ky+1)*dimX_half + kx*double_v::size() + 1], VcEA);
          red[ky+1][kx]   = double_v(&grid_r[idx + (ky+2)*dimX_half + kx*double_v::size() + 1], VcEA);
          rhs[ky+1][kx]   = double_v( &rhs_r[idx + (ky+2)*dimX_half + kx*double_v::size() + 1], VcEA);
          blk_l[ky+2][kx] = double_v(&grid_b[idx + (ky+2)*dimX_half + kx*double_v::size()    ], VcEA);
          blk_r[ky+1][kx] = double_v(&grid_b[idx + (ky+2)*dimX_half + kx*double_v::size() + 1], VcEA);
        }
      }
      for (int ky = 0; ky < 2*BLOCKSIZEY; ky+=2) {
        for (int kx = 0; kx < BLOCKSIZEX; ++kx) {
          rhs[ky  ][kx] -= DiffusionKernel(red[ky  ][kx], blk_l[ky+2][kx], blk_l[ky][kx], 
                                                          blk_l[ky+1][kx], blk_r[ky][kx]);
          red[ky  ][kx] -= pre * rhs[ky][kx];
          rhs[ky+1][kx] -= DiffusionKernel(red[ky+1][kx], blk_r[ky+2][kx], blk_r[ky  ][kx], 
                                                          blk_l[ky+2][kx], blk_r[ky+1][kx]);
          red[ky+1][kx] -= pre * rhs[ky+1][kx];
        }
      }
      for (int ky = 0; ky < 2*BLOCKSIZEY; ky+=2) {
        for (int kx = 0; kx < BLOCKSIZEX; ++kx) {
          red[ky  ][kx].memstore(&grid_r[idx + (ky+1)*dimX_half + kx*double_v::size()    ], VcEA);
          red[ky+1][kx].memstore(&grid_r[idx + (ky+2)*dimX_half + kx*double_v::size() + 1], VcEA);
        }
      }
    }
  }
  RBSORUpdate<dimX_half, RED>(grid_r);
#pragma omp parallel for num_threads(NUMTHREADS)
  for (int y = 0; y < dimY; y += 2*BLOCKSIZEY) {
    for (int x = 0; x < dimX/2; x += BLOCKSIZEX*double_v::size()) {
      int idx = y*dimX_half + x;
      std::array<std::array<double_v, BLOCKSIZEX>, 2*BLOCKSIZEY> red;
      std::array<std::array<double_v, BLOCKSIZEX>, 2*BLOCKSIZEY> rhs;
      std::array<std::array<double_v, BLOCKSIZEX>, 2*BLOCKSIZEY + 1> blk_l;
      std::array<std::array<double_v, BLOCKSIZEX>, 2*BLOCKSIZEY + 1> blk_r;
      for (int kx = 0; kx < BLOCKSIZEX; ++kx) {
        blk_l[2*BLOCKSIZEY][kx] = double_v(&grid_r[idx + (2*BLOCKSIZEY+1)*dimX_half + kx*double_v::size()], VcEA);
        blk_r[0           ][kx] = double_v(&grid_r[idx + kx*double_v::size() + 1], VcEA);
      }
      for (int ky = 0; ky < 2*BLOCKSIZEY; ky+=2) {
        for (int kx = 0; kx < BLOCKSIZEX; ++kx) {
          red[ky  ][kx]   = double_v(&grid_b[idx + (ky+1)*dimX_half + kx*double_v::size() + 1], VcEA);
          rhs[ky  ][kx]   = double_v( &rhs_b[idx + (ky+1)*dimX_half + kx*double_v::size() + 1], VcEA);
          blk_l[ky  ][kx] = double_v(&grid_r[idx + (ky+1)*dimX_half + kx*double_v::size()    ], VcEA);
          blk_r[ky+1][kx] = double_v(&grid_r[idx + (ky+1)*dimX_half + kx*double_v::size() + 1], VcEA);
          red[ky+1][kx]   = double_v(&grid_b[idx + (ky+2)*dimX_half + kx*double_v::size()    ], VcEA);
          rhs[ky+1][kx]   = double_v( &rhs_b[idx + (ky+2)*dimX_half + kx*double_v::size()    ], VcEA);
          blk_l[ky+1][kx] = double_v(&grid_r[idx + (ky+2)*dimX_half + kx*double_v::size()    ], VcEA);
          blk_r[ky+2][kx] = double_v(&grid_r[idx + (ky+2)*dimX_half + kx*double_v::size() + 1], VcEA);
        }
      }
      for (int ky = 0; ky < 2*BLOCKSIZEY; ky+=2) {
        for (int kx = 0; kx < BLOCKSIZEX; ++kx) {
          rhs[ky  ][kx] -= DiffusionKernel(red[ky  ][kx], blk_r[ky+2][kx], blk_r[ky][kx], 
                                                          blk_l[ky][kx], blk_r[ky+1][kx]);
          red[ky  ][kx] -= pre * rhs[ky][kx];
          rhs[ky+1][kx] -= DiffusionKernel(red[ky+1][kx], blk_l[ky+2][kx], blk_l[ky  ][kx], 
                                                          blk_l[ky+1][kx], blk_r[ky+2][kx]);
          red[ky+1][kx] -= pre * rhs[ky+1][kx];
        }
      }
      for (int ky = 0; ky < 2*BLOCKSIZEY; ky+=2) {
        for (int kx = 0; kx < BLOCKSIZEX; ++kx) {
          red[ky  ][kx].memstore(&grid_b[idx + (ky+1)*dimX_half + kx*double_v::size() + 1], VcEA);
          red[ky+1][kx].memstore(&grid_b[idx + (ky+2)*dimX_half + kx*double_v::size()    ], VcEA);
        }
      }
    }
  }
  RBSORUpdate<dimX_half, BLACK>(grid_b);
}

// iter*(dimX*dimY*12)  + iter/INNER_LOOP*(dimX*dimY*12)  
extern "C" double SORDiffusion(std::vector<double> &grid_r, std::vector<double> &grid_b, 
                               const std::vector<double> &rhs_r, const std::vector<double> &rhs_b, 
                               const double &eps, double &res, double &rate, double &time, size_t &iter,
                               size_t &itermax) {
  std::cout << OMEGA << ", " << BLOCKSIZEX << ", " << BLOCKSIZEY << ", " << NUMTHREADS << std::endl;
  const double eps_limit = eps*eps*dimX*dimY;
  iter = 0;
  size_t iter_top = 0;
  double res_old = -1.0;
  double res_top = -1.0;

  for (int y = 1; y <= dimY; y += 1) {
    for (int x = 0; x <= dimX/2; x += 1) {
      int i = (y)*(dimX/2 + 1) + x;
      grid_r[i] = 0.0;
      grid_b[i] = 0.0;
    }
  }

  res = 0.0;
  
  auto start = std::chrono::high_resolution_clock::now();
  while ((res = RBSORResidual(grid_r.data(), grid_b.data(), rhs_r.data(), rhs_b.data())) > eps_limit) {
    if (res > res_top) {
      res_top = res;
      iter_top = iter;
    }
    if (iter > ITER_MIN) {
      if (iter > 2*itermax) break;
      if (res_top <= res + eps) break;
      if (abs(res_old - res) < eps) break;
    }
    res_old = res;
    
    for (int i = 0; i < INNER_LOOP; ++i) {
      ++iter;
      RBSORCycle(grid_r.data(), grid_b.data(), rhs_r.data(), rhs_b.data());
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;
  
  if (iter < itermax) itermax = iter;
  
  time = diff.count();
  rate = pow(res_top/res, 0.5/(iter - iter_top)) - 1.0;
  res  = sqrt(res/(dimX*dimY));
  return res;
}
