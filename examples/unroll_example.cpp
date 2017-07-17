#include "opttmp/unroll_loop.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

int main(void) {
  constexpr size_t N = 100;

  std::vector<double> arr(N);
  std::fill(arr.begin(), arr.end(), 0.0);

  opttmp::loop::unroll_loop<10, 40, 2>([&arr](auto i) { arr[i] = 3.0; });

  for (size_t i = 0; i < N; i++) {
    std::cout << "i: " << i << " -> " << arr[i] << std::endl;
  }

  return 0;
}
