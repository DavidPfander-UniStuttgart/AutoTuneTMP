#include <vector>

#include "opttmp/loop/unroll_loop.hpp"

#include "parameters.hpp"

extern "C" void unrolling_kernel(std::vector<double> &arr, const size_t N) {
  opttmp::loop::unroll_loop<6, 14, 2>([&arr](auto i) { arr[i] = 3.0; });
  std::cout << "UNROLL_LOOP is: " << UNROLL_LOOP << std::endl;
#if UNROLL_LOOP == 1
  std::cout << "UNROLL_LOOP is defined!" << std::endl;
#endif
}
