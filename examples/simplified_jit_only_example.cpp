#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>

using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;
using namespace std; // to shorten code

#include "autotune/autotune.hpp"
using namespace autotune;
using namespace cppjit::builder;

// defines kernel, put in single compilation unit
AUTOTUNE_DECLARE_DEFINE_KERNEL(void(vector<double> &, vector<double> &), square)

int main(void) {
  size_t N = 2048 * 2048; // 32MB
  vector<double> origin(N, 2.0);
  vector<double> dest(N);

  square.set_source_dir("examples/jit_only_kernel");
  square.get_builder<gcc>().set_cpp_flags("-std=c++17 -O3 "
                                          "-march=native -mtune=native");
  // compiled before first run
  square(origin, dest);

  std::cout << "origin[5]: " << origin[5] << std::endl;
  std::cout << "dest[5]: " << dest[5] << std::endl;
}
