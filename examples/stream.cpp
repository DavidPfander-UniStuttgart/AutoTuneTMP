#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>

#include "autotune/autotune.hpp"
#include "autotune/fixed_set_parameter.hpp"
#include "autotune/tuners/line_search.hpp"

using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;

// defines kernel, put in single compilation unit
AUTOTUNE_KERNEL(void(std::vector<double> &, std::vector<double> &), copy,
                "examples/stream_kernel")
// AUTOTUNE_KERNEL(void(std::vector<double> &, std::vector<double> &, double),
//                 scale, "examples/stream_kernel")
// AUTOTUNE_KERNEL(void(std::vector<double> &, std::vector<double> &,
//                      std::vector<double> &),
//                 sum, "examples/stream_kernel")
// AUTOTUNE_KERNEL(void(std::vector<double> &, std::vector<double> &,
//                      std::vector<double> &, double),
//                 triad, "examples/stream_kernel")

int main(void) {
  autotune::copy.set_verbose(true);
  auto &builder = autotune::copy.get_builder<cppjit::builder::gcc>();
  builder.set_cpp_flags("-Wall -Wextra -fopenmp -std=c++17 -O3 -g "
                        "-march=native -mtune=native -fstrict-aliasing ");
  builder.set_link_flags("-std=c++17 -O3 -g -fopenmp -fstrict-aliasing ");
  builder.set_do_cleanup(false);

  autotune::countable_set parameters;
  autotune::fixed_set_parameter<size_t> p1("KERNEL_OMP_THREADS", {2});
  parameters.add_parameter(p1);
  autotune::copy.set_parameter_values(parameters);

  // register parameters
  // autotune::countable_set parameters;
  // autotune::fixed_set_parameter<std::string> p1("ADD_ONE", {"0", "1"},
  // false); parameters.add_parameter(p1); int a = 5;

  // size_t line_search_iterations = 1;
  // autotune::tuners::line_search tuner(autotune::add_one, parameters,
  //                                     line_search_iterations);
  // autotune::countable_set optimal_parameters = tuner.tune(a);
  // autotune::add_one.set_parameter_values(optimal_parameters);

  size_t N = 100000000;
  std::vector<double> a(N, 1.0);
  std::vector<double> b(N, 2.0);
  std::vector<double> c(N, 3.0);
  double q = 5.0;

  size_t repeat = 100;

  time_point start = high_resolution_clock::now();

  for (size_t r = 0; r < repeat; r += 1) {
    autotune::copy(a, b);
  }
  time_point end = high_resolution_clock::now();
  double duration_s = std::chrono::duration<double>(end - start).count();
  double duration_s_avr = duration_s / repeat;
  double bytes_copied = 8.0 * 3.0 * N;
  double bandwidth_gb = (bytes_copied / duration_s_avr) * 1E-9;
  std::cout << "N: " << N << std::endl;
  std::cout << "repeat: " << repeat << std::endl;
  std::cout << " 2 read, 1 write: (8B * 3 * N / (t / repeat)) * 1E-9 "
               "-> achieved "
               "bandwidth GB/s: "
            << bandwidth_gb << std::endl;
  std::cout << "not using non-temporal stores, with those only 1 read, 1 write"
            << std::endl;

  // bool ok = std::equal(a.begin(), a.end(), b.begin());
  // if (!ok) {
  //   throw;
  // }
  return 0;
}
