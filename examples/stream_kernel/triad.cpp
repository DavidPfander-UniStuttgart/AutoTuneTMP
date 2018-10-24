#include "parameters.hpp"
#include <iostream>
#include <omp.h>
#include <vector>

AUTOTUNE_EXPORT int triad(std::vector<double> &a, std::vector<double> &b,
                          std::vector<double> &c, double q) {
  omp_set_num_threads(KERNEL_OMP_THREADS);
  // std::cout << "KERNEL_OMP_THREADS: " << KERNEL_OMP_THREADS << std::endl;

  const size_t N = a.size();
#pragma omp parallel for
  for (size_t i = 0; i < N; i += 1) {
    a[i] = b[i] + q * c[i];
  }
}
