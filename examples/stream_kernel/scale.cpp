#include "parameters.hpp"

AUTOTUNE_EXPORT int scale(std::vector<double> &a, std::vector<double> &b,
                          double q) {
  size_t N = a.size();
  for (size_t i = 0; i < N; i += 1) {
    a[i] = q * b[i];
  }
}
