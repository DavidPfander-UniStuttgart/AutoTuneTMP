#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>

// #include <boost/align/aligned_allocator.hpp>
// using boost::alignment::aligned_allocator;
using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;

#include "autotune/autotune.hpp"

// defines kernel, put in single compilation unit
AUTOTUNE_DECLARE_DEFINE_KERNEL(void(std::vector<double> &,
                                    std::vector<double> &),
                               square)

int main(void) {

  auto builder = autotune::square.get_builder_as<cppjit::builder::gcc>();
  builder->set_verbose(true);

  size_t N = 2048 * 2048; // 32MB
  size_t repetitions = 100;
  std::vector<double> origin(N);
  std::fill(origin.begin(), origin.end(), 2.0);
  std::vector<double> dest_slow(N);
  std::vector<double> dest_fast(N);

  std::vector<double> dest_reference(N);
  std::fill(dest_reference.begin(), dest_reference.end(), 4.0);

  autotune::square.set_source_dir("examples/jit_only_kernel");

  double duration_slow = 0.0;

  {
    builder->set_cpp_flags("-std=c++17");
    autotune::square.compile();

    for (size_t i = 0; i < repetitions; i++) {
      time_point start = high_resolution_clock::now();
      autotune::square(origin, dest_slow);
      time_point end = high_resolution_clock::now();
      duration_slow += std::chrono::duration<double>(end - start).count();
    }
    std::cout << "avr. copy duration (native not specified): "
              << (duration_slow / repetitions) << std::endl;
    bool is_eqal = std::equal(dest_reference.begin(), dest_reference.end(),
                              dest_slow.begin());
    if (is_eqal) {
      std::cout << "ranges are equal!" << std::endl;
    } else {
      std::cout << "ranges are NOT equal!" << std::endl;
    }
  }

  double duration_fast = 0.0;
  {
    builder->set_cpp_flags("-std=c++17 -O3 -march=native -mtune=native");
    autotune::square.compile();
    for (size_t i = 0; i < repetitions; i++) {
      time_point start = high_resolution_clock::now();
      autotune::square(origin, dest_fast);
      time_point end = high_resolution_clock::now();
      duration_fast += std::chrono::duration<double>(end - start).count();
    }
    std::cout << "avr. copy duration (native specified): "
              << (duration_fast / repetitions) << std::endl;
    bool is_eqal = std::equal(dest_reference.begin(), dest_reference.end(),
                              dest_fast.begin());
    if (is_eqal) {
      std::cout << "ranges are equal!" << std::endl;
    } else {
      std::cout << "ranges are NOT equal!" << std::endl;
    }
  }

  std::cout << "avr. speedup (slow / fast): " << (duration_slow / duration_fast)
            << "x" << std::endl;

  return 0;
}
