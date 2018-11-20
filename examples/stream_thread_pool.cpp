#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>

#include "autotune/autotune.hpp"
#include "autotune/fixed_set_parameter.hpp"
#include "autotune/tuners/line_search.hpp"
#include "autotune/queue_thread_pool.hpp"

#include <boost/align/aligned_allocator.hpp>

using align = boost::alignment::aligned_allocator<double, 64>;

using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;

// defines kernel, put in single compilation unit
AUTOTUNE_KERNEL(void(size_t, size_t),
                copy, "examples/stream_kernel_thread_pool")
AUTOTUNE_KERNEL(void(size_t, size_t),
                scale, "examples/stream_kernel_thread_pool")
AUTOTUNE_KERNEL(void(size_t, size_t),
                sum, "examples/stream_kernel_thread_pool")
AUTOTUNE_KERNEL(void(size_t, size_t),
                triad, "examples/stream_kernel_thread_pool")

int main(void) {

  autotune::countable_set parameters;
  autotune::fixed_set_parameter<size_t> p1("KERNEL_THREADS", {16});
  parameters.add_parameter(p1);
  autotune::fixed_set_parameter<size_t> p2("REG_BLOCKING", {1});
  parameters.add_parameter(p2);
  autotune::fixed_set_parameter<uint64_t> p3("AFFINITY_POLICY", {static_cast<uint64_t>(autotune::affinity_type_t::sparse)});
  parameters.add_parameter(p3);  

  {
    autotune::copy.set_verbose(true);
    auto &builder = autotune::copy.get_builder<cppjit::builder::gcc>();
    builder.set_cpp_flags("-Wall -Wextra -fopenmp -std=c++17 -O3 " // -fno-exceptions -fwrapv
			  "-funsafe-math-optimizations -g "
                          "-march=native -mtune=native -fstrict-aliasing ");
    builder.set_include_paths(
        "-Ilikwid/src/includes -Iboost_install/include -IVc_install/include -Iinclude");
    builder.set_link_flags("-std=c++17 -O3 -g -fopenmp -fstrict-aliasing ");
    builder.set_library_paths("-Lboost_install/lib -Llikwid -Llikwid/ext/hwloc -Llikwid/ext/lua");
    builder.set_libraries("-lboost_thread -llikwid -llikwid-lua -llikwid-hwloc");
    builder.set_do_cleanup(false);
    autotune::copy.set_parameter_values(parameters);
    autotune::copy.compile();
  }

  {
    autotune::scale.set_verbose(true);
    auto &builder = autotune::scale.get_builder<cppjit::builder::gcc>();
    builder.set_cpp_flags("-Wall -Wextra -fopenmp -std=c++17 -O3 -funsafe-math-optimizations -g "
                          "-march=native -mtune=native -fstrict-aliasing ");
    builder.set_include_paths(
        "-Ilikwid/src/includes -Iboost_install/include -IVc_install/include -Iinclude");
    builder.set_link_flags("-std=c++17 -O3 -g -fopenmp -fstrict-aliasing ");
    builder.set_library_paths("-Lboost_install/lib -Llikwid -Llikwid/ext/hwloc -Llikwid/ext/lua");
    builder.set_libraries("-lboost_thread -llikwid -llikwid-lua -llikwid-hwloc");
    builder.set_do_cleanup(false);
    autotune::scale.set_parameter_values(parameters);
    autotune::scale.compile();
  }

  {
    autotune::sum.set_verbose(true);
    auto &builder = autotune::sum.get_builder<cppjit::builder::gcc>();
    builder.set_cpp_flags("-Wall -Wextra -fopenmp -std=c++17 -O3  -funsafe-math-optimizations -g "
                          "-march=native -mtune=native -fstrict-aliasing ");
    builder.set_include_paths(
        "-Ilikwid/src/includes -Iboost_install/include -IVc_install/include -Iinclude");
    builder.set_link_flags("-std=c++17 -O3 -g -fopenmp -fstrict-aliasing ");
    builder.set_library_paths("-Lboost_install/lib -Llikwid -Llikwid/ext/hwloc -Llikwid/ext/lua");
    builder.set_libraries("-lboost_thread -llikwid -llikwid-lua -llikwid-hwloc");    
    builder.set_do_cleanup(false);
    autotune::sum.set_parameter_values(parameters);
    autotune::sum.compile();
  }

  {
    autotune::triad.set_verbose(true);
    auto &builder = autotune::triad.get_builder<cppjit::builder::gcc>();
    builder.set_cpp_flags(
        "-Wall -Wextra -fopenmp -std=c++17 -O3 -funsafe-math-optimizations -g "
        "-march=native -mtune=native -fstrict-aliasing -Iinclude");
    builder.set_include_paths("-Ilikwid/src/includes -Iboost_install/include -IVc_install/include -Iinclude");
    builder.set_link_flags("-std=c++17 -O3 -g -fopenmp -fstrict-aliasing ");
    builder.set_library_paths("-Lboost_install/lib -Llikwid -Llikwid/ext/hwloc -Llikwid/ext/lua");
    builder.set_libraries("-lboost_thread -llikwid -llikwid-lua -llikwid-hwloc");
    builder.set_do_cleanup(false);
    autotune::triad.set_parameter_values(parameters);
    autotune::triad.compile();
  }

  size_t N_per_task = 200000000 / 8;
  // adjust to next number divisible by 64
  if (N_per_task % 64 != 0) {
    N_per_task += 64 - (N_per_task % 64);
  }
  size_t repeat = 200;
  std::cout << "warning: not using non-temporal stores" << std::endl;
  autotune::copy(N_per_task, repeat);
  autotune::scale(N_per_task, repeat);
  autotune::sum(N_per_task, repeat);
  autotune::triad(N_per_task, repeat);

  return 0;
}
