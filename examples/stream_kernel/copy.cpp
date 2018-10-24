#include "parameters.hpp"
#include <boost/align/aligned_allocator.hpp>
#include <iostream>
#include <omp.h>
#include <vector>

using align = boost::alignment::aligned_allocator<double, 64>;

AUTOTUNE_EXPORT void copy(std::vector<double, align> &a,
                          std::vector<double, align> &b) {

  omp_set_num_threads(KERNEL_OMP_THREADS);
  // std::cout << "KERNEL_OMP_THREADS: " << KERNEL_OMP_THREADS << std::endl;

  const size_t N = a.size();
#pragma omp parallel for
  for (size_t i = 0; i < N; i += 1) {
    a[i] = b[i];
  }
}
