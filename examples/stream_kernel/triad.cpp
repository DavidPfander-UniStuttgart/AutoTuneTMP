#include "parameters.hpp"

AUTOTUNE_EXPORT int sum(std::vector<double> &a, std::vector<double> &b,
                        std::vector<double> &b, double q) {
  size_t N = a.size();
  for (size_t i = 0; i < N; i += 1) {
    a[i] = b[i] + q * c[i];
  }
}
