#include "opttmp/vectorization/register_tiling.hpp"
#include "parameters.hpp"
#include <Vc/Vc>
#include <boost/align/aligned_allocator.hpp>
#include <iostream>
#include <omp.h>
#include <vector>

using reg_arr =
    opttmp::vectorization::register_array<Vc::double_v, REG_BLOCKING>;
using align = boost::alignment::aligned_allocator<double, 64>;

AUTOTUNE_EXPORT void scale(std::vector<double, align> &a,
                           std::vector<double, align> &b, double q) {

  omp_set_num_threads(KERNEL_OMP_THREADS);
  // std::cout << "KERNEL_OMP_THREADS: " << KERNEL_OMP_THREADS << std::endl;

  // not kept in registers for unknown reasons
  const Vc::double_v q_vec = q;
  // std::cout << "q_vec: " << q_vec << std::endl;
  const size_t N = a.size();
#pragma omp parallel for
  for (size_t i = 0; i < N; i += REG_BLOCKING * Vc::double_v::size()) {
    // vector_aligned leads to crash?
    reg_arr temp(&b[i], Vc::flags::element_aligned);
    temp = q_vec * temp;
    temp.memstore(&a[i], Vc::flags::element_aligned);
    // a[i] = q * b[i];
  }
}
