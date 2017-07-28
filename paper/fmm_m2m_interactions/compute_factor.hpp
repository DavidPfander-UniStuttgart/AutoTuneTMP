#pragma once

#include "types.hpp"
#include "taylor.hpp"
#include "kernels/m2m_simd_types.hpp"

extern taylor<4, real> factor;
extern taylor<4, m2m_vector> factor_half_v;
extern taylor<4, m2m_vector> factor_sixth_v;

void compute_factor();
