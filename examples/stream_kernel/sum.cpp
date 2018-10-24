#include "opttmp/vectorization/register_tiling.hpp"
#include "parameters.hpp"
#include <boost/align/aligned_allocator.hpp>
#include <iostream>
#include <omp.h>
#include <vector>

using reg_arr =
    opttmp::vectorization::register_array<Vc::double_v, REG_BLOCKING>;
using align = boost::alignment::aligned_allocator<double, 64>;

AUTOTUNE_EXPORT void sum(std::vector<double, align> &a,
                         std::vector<double, align> &b,
                         std::vector<double, align> &c) {
  omp_set_num_threads(KERNEL_OMP_THREADS);
  // std::cout << "KERNEL_OMP_THREADS: " << KERNEL_OMP_THREADS << std::endl;

  const size_t N = a.size();
#pragma omp parallel for
  for (size_t i = 0; i < N; i += REG_BLOCKING * Vc::double_v::size()) {
    reg_arr temp(&b[i], Vc::flags::element_aligned);
    reg_arr temp2(&c[i], Vc::flags::element_aligned);
    temp += temp2;
    temp.memstore(&a[i], Vc::flags::element_aligned);
    // a[i] = b[i] + c[i];
  }
}
