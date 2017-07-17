#include "opttmp/loop/loop_nest.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

int main(void) {
  constexpr size_t N = 10;

  std::vector<double> arr(N * N);
  std::fill(arr.begin(), arr.end(), 0.0);

  opttmp::loop::loop_nest<2, double>(
      {0, 0}, {N, N}, {1, 1}, [&arr](auto (&i)[2]) { arr[i[0] * N + i[1]] = 3.0; });

  for (size_t i = 0; i < N * N; i++) {
    std::cout << "i: " << i << " -> " << arr[i] << std::endl;
  }

  return 0;
}
