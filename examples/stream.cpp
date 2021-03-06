#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>

#include "autotune/autotune.hpp"
#include "autotune/fixed_set_parameter.hpp"
#include "autotune/tuners/line_search.hpp"

#include <boost/align/aligned_allocator.hpp>

using align = boost::alignment::aligned_allocator<double, 64>;

using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;

// defines kernel, put in single compilation unit
AUTOTUNE_KERNEL(void(std::vector<double, align> &,
                     std::vector<double, align> &),
                copy, "examples/stream_kernel")
AUTOTUNE_KERNEL(void(std::vector<double, align> &, std::vector<double, align> &,
                     double),
                scale, "examples/stream_kernel")
AUTOTUNE_KERNEL(void(std::vector<double, align> &, std::vector<double, align> &,
                     std::vector<double, align> &),
                sum, "examples/stream_kernel")
AUTOTUNE_KERNEL(void(std::vector<double, align> &, std::vector<double, align> &,
                     std::vector<double, align> &, double),
                triad, "examples/stream_kernel")

void print_bandwidth(size_t N, size_t repeat, double bytes_traffic,
                     double duration_s_avr) {
  double bandwidth_gb = (bytes_traffic / duration_s_avr) * 1E-9;
  std::cout << "bandwidth GB/s: " << bandwidth_gb << std::endl;
}

int main(void) {

  autotune::countable_set parameters;
  autotune::fixed_set_parameter<size_t> p1("KERNEL_OMP_THREADS", {2});
  parameters.add_parameter(p1);
  autotune::fixed_set_parameter<size_t> p2("REG_BLOCKING", {1});
  parameters.add_parameter(p2);

  {
    autotune::copy.set_verbose(true);
    auto &builder = autotune::copy.get_builder<cppjit::builder::gcc>();
    builder.set_cpp_flags("-Wall -Wextra -fopenmp -std=c++17 -O3 -g "
                          "-march=native -mtune=native -fstrict-aliasing ");
    builder.set_include_paths(
        "-Iboost_install/include -IVc_install/include -Iinclude");
    builder.set_link_flags("-std=c++17 -O3 -g -fopenmp -fstrict-aliasing ");
    builder.set_do_cleanup(false);
    autotune::copy.set_parameter_values(parameters);
    autotune::copy.compile();
  }

  {
    autotune::scale.set_verbose(true);
    auto &builder = autotune::scale.get_builder<cppjit::builder::gcc>();
    builder.set_cpp_flags("-Wall -Wextra -fopenmp -std=c++17 -O3 -g "
                          "-march=native -mtune=native -fstrict-aliasing ");
    builder.set_include_paths(
        "-Iboost_install/include -IVc_install/include -Iinclude");
    builder.set_link_flags("-std=c++17 -O3 -g -fopenmp -fstrict-aliasing ");
    builder.set_do_cleanup(false);
    autotune::scale.set_parameter_values(parameters);
    autotune::scale.compile();
  }

  {
    autotune::sum.set_verbose(true);
    auto &builder = autotune::sum.get_builder<cppjit::builder::gcc>();
    builder.set_cpp_flags("-Wall -Wextra -fopenmp -std=c++17 -O3 -g "
                          "-march=native -mtune=native -fstrict-aliasing ");
    builder.set_include_paths(
        "-Iboost_install/include -IVc_install/include -Iinclude");
    builder.set_link_flags("-std=c++17 -O3 -g -fopenmp -fstrict-aliasing ");
    builder.set_do_cleanup(false);
    autotune::sum.set_parameter_values(parameters);
    autotune::sum.compile();
  }

  {
    autotune::triad.set_verbose(true);
    auto &builder = autotune::triad.get_builder<cppjit::builder::gcc>();
    builder.set_cpp_flags(
        "-Wall -Wextra -fopenmp -std=c++17 -O3 -g "
        "-march=native -mtune=native -fstrict-aliasing -Iinclude");
    builder.set_include_paths("-Iboost_install/include -IVc_install/include");
    builder.set_link_flags("-std=c++17 -O3 -g -fopenmp -fstrict-aliasing ");
    builder.set_do_cleanup(false);
    autotune::triad.set_parameter_values(parameters);
    autotune::triad.compile();
  }

  size_t N = 1000000000 / 8;
  // adjust to next number divisible by 64
  if (N % 64 != 0) {
    N += 64 - (N % 64);
  }
  std::cout << "N: " << N << " -> " << (static_cast<double>(8 * N) * 1E-9)
            << "GB" << std::endl;
  std::vector<double, align> a(N, 1.0);
  std::vector<double, align> b(N, 2.0);
  std::vector<double, align> c(N, 3.0);
  double q = 5.0;

  size_t repeat = 10;
  std::cout << "warning: not using non-temporal stores" << std::endl;

  {
    time_point start = high_resolution_clock::now();
    for (size_t r = 0; r < repeat; r += 1) {
      autotune::copy(a, b);
    }
    time_point end = high_resolution_clock::now();
    double duration_s_avr =
        std::chrono::duration<double>(end - start).count() / repeat;
    double bytes_copied = 8.0 * 3.0 * N;
    std::cout << "copy: 2 read, 1 write" << std::endl;
    print_bandwidth(N, repeat, bytes_copied, duration_s_avr);
  }

  {
    time_point start = high_resolution_clock::now();
    for (size_t r = 0; r < repeat; r += 1) {
      autotune::scale(a, b, q);
    }
    time_point end = high_resolution_clock::now();
    double duration_s_avr =
        std::chrono::duration<double>(end - start).count() / repeat;
    double bytes_copied = 8.0 * 3.0 * N;
    std::cout << "scale: 2 read, 1 write" << std::endl;
    print_bandwidth(N, repeat, bytes_copied, duration_s_avr);
  }

  {
    time_point start = high_resolution_clock::now();
    for (size_t r = 0; r < repeat; r += 1) {
      autotune::sum(a, b, c);
    }
    time_point end = high_resolution_clock::now();
    double duration_s_avr =
        std::chrono::duration<double>(end - start).count() / repeat;
    double bytes_copied = 8.0 * 4.0 * N;
    std::cout << "sum: 2 read, 2 write" << std::endl;
    print_bandwidth(N, repeat, bytes_copied, duration_s_avr);
  }

  {
    time_point start = high_resolution_clock::now();
    for (size_t r = 0; r < repeat; r += 1) {
      autotune::triad(a, b, c, q);
    }
    time_point end = high_resolution_clock::now();
    double duration_s_avr =
        std::chrono::duration<double>(end - start).count() / repeat;
    double bytes_copied = 8.0 * 4.0 * N;
    std::cout << "triad 2 read, 2 write" << std::endl;
    print_bandwidth(N, repeat, bytes_copied, duration_s_avr);
  }

  return 0;
}
