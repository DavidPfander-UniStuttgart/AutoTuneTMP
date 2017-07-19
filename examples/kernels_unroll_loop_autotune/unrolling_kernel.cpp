#include "opttmp/loop/unroll_loop.hpp"
#include <vector>
extern "C" void unrolling_kernel(std::vector<double> &arr, const size_t N) {
  opttmp::loop::unroll_loop<6, 14, 2>([&arr](auto i) { arr[i] = 3.0; });
  for (size_t i = 0; i < N; i++) {
    std::cout << "i: " << i << " -> " << arr[i] << std::endl;
  }
}
