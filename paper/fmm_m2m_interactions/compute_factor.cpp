#include "kernels/m2m_simd_types.hpp"
#include "taylor.hpp"

taylor<4, real> factor;
taylor<4, m2m_vector> factor_half_v;
taylor<4, m2m_vector> factor_sixth_v;

void compute_factor() {
  factor = 0.0;
  factor() += 1.0;
  for (integer a = 0; a < NDIM; ++a) {
    factor(a) += 1.0;
    for (integer b = 0; b < NDIM; ++b) {
      factor(a, b) += 1.0;
      for (integer c = 0; c < NDIM; ++c) {
        factor(a, b, c) += 1.0;
      }
    }
  }

  const m2m_vector half_v(1.0 / 2.0);
  const m2m_vector sixth_v(1.0 / 6.0);
  for (size_t i = 0; i < factor.size(); i++) {
    factor_half_v[i] = half_v * factor[i];
    factor_sixth_v[i] = sixth_v * factor[i];
  }
}

// initialize constants at application startup
namespace detail {
struct init_factor {
  init_factor() {
    std::cout << "initializing factor" << std::endl;
    compute_factor();
  }
} init_factor_dummy;
}
