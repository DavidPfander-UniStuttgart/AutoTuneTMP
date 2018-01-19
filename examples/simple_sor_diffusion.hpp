#ifndef SIMPLE_SOR_DEFFUSION_HPP
#define SIMPLE_SOR_DEFFUSION_HPP

const int dimX = 512;
const int dimX_half = (dimX/2) + 1;
const int dimY = 512;
const double eps = 1e-3;
const double hx = 2.0*M_PI/dimX;
const double hy = 2.0*M_PI/dimY;

#define INNER_LOOP 10
#define ITER_MIN 256

#endif
