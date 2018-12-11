#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "autotune/autotune.hpp"
#include "autotune/fixed_set_parameter.hpp"
#include "autotune/log_parameter.hpp"
#include "autotune/queue_thread_pool.hpp"
#include "autotune/tuners/parallel_line_search.hpp"
#include "autotune/tuners/parallel_neighborhood_search.hpp"
#include "opttmp/numa_topology.hpp"

#include <boost/align/aligned_allocator.hpp>

using align = boost::alignment::aligned_allocator<double, 64>;

using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;

// defines kernel, put in single compilation unit
// AUTOTUNE_KERNEL(void(size_t, size_t, double &), copy,
//                 "examples/stream_kernel_thread_pool")
// AUTOTUNE_KERNEL(void(size_t, size_t, double &), scale,
//                 "examples/stream_kernel_thread_pool")
// AUTOTUNE_KERNEL(void(size_t, size_t, double &), sum,
//                 "examples/stream_kernel_thread_pool")
AUTOTUNE_KERNEL(void(size_t, size_t, double &), triad,
                "examples/stream_kernel_thread_pool")

int main(void) {

  std::string hostname(getenv("HOSTNAME"));

  opttmp::numa_topology_t numa_topology;
  uint32_t cores_total = numa_topology.get_cores_total();

  autotune::countable_set parameters;
  // 2^0=1 ... 2^6=64 cores
  autotune::log_parameter p1("KERNEL_THREADS", 0, 2, 0,
                             std::log2(static_cast<double>(cores_total)),
                             1); // {1, 2, 4, 8, 16}
  parameters.add_parameter(p1);
  autotune::fixed_set_parameter<size_t> p2("REG_BLOCKING", {1, 2, 4});
  parameters.add_parameter(p2);
  autotune::fixed_set_parameter<uint64_t> p3(
      "AFFINITY_POLICY",
      {static_cast<uint64_t>(autotune::affinity_type_t::sparse),
       static_cast<uint64_t>(autotune::affinity_type_t::compact)});
  parameters.add_parameter(p3);

  // {
  //   autotune::copy.set_verbose(true);
  //   auto &builder = autotune::copy.get_builder<cppjit::builder::gcc>();
  //   builder.set_cpp_flags(
  //       "-Wall -Wextra -fopenmp -std=c++17 -O3 " // -fno-exceptions -fwrapv
  //       "-funsafe-math-optimizations -g "
  //       "-march=native -mtune=native -fstrict-aliasing ");
  //   builder.set_include_paths("-Ilikwid/src/includes -Iboost_install/include
  //   "
  //                             "-IVc_install/include -Iinclude");
  //   builder.set_link_flags("-std=c++17 -O3 -g -fopenmp -fstrict-aliasing ");
  //   builder.set_library_paths(
  //       "-Lboost_install/lib -Llikwid -Llikwid/ext/hwloc -Llikwid/ext/lua");
  //   builder.set_libraries(
  //       "-lboost_thread -llikwid -llikwid-lua -llikwid-hwloc");
  //   builder.set_do_cleanup(false);
  //   autotune::copy.set_parameter_values(parameters);
  //   autotune::copy.compile();
  // }

  // {
  //   autotune::scale.set_verbose(true);
  //   auto &builder = autotune::scale.get_builder<cppjit::builder::gcc>();
  //   builder.set_cpp_flags(
  //       "-Wall -Wextra -fopenmp -std=c++17 -O3 -funsafe-math-optimizations -g
  //       "
  //       "-march=native -mtune=native -fstrict-aliasing ");
  //   builder.set_include_paths("-Ilikwid/src/includes -Iboost_install/include
  //   "
  //                             "-IVc_install/include -Iinclude");
  //   builder.set_link_flags("-std=c++17 -O3 -g -fopenmp -fstrict-aliasing ");
  //   builder.set_library_paths(
  //       "-Lboost_install/lib -Llikwid -Llikwid/ext/hwloc -Llikwid/ext/lua");
  //   builder.set_libraries(
  //       "-lboost_thread -llikwid -llikwid-lua -llikwid-hwloc");
  //   builder.set_do_cleanup(false);
  //   autotune::scale.set_parameter_values(parameters);
  //   autotune::scale.compile();
  // }

  // {
  //   autotune::sum.set_verbose(true);
  //   auto &builder = autotune::sum.get_builder<cppjit::builder::gcc>();
  //   builder.set_cpp_flags(
  //       "-Wall -Wextra -fopenmp -std=c++17 -O3  -funsafe-math-optimizations
  //       -g "
  //       "-march=native -mtune=native -fstrict-aliasing ");
  //   builder.set_include_paths("-Ilikwid/src/includes -Iboost_install/include
  //   "
  //                             "-IVc_install/include -Iinclude");
  //   builder.set_link_flags("-std=c++17 -O3 -g -fopenmp -fstrict-aliasing ");
  //   builder.set_library_paths(
  //       "-Lboost_install/lib -Llikwid -Llikwid/ext/hwloc -Llikwid/ext/lua");
  //   builder.set_libraries(
  //       "-lboost_thread -llikwid -llikwid-lua -llikwid-hwloc");
  //   builder.set_do_cleanup(false);
  //   autotune::sum.set_parameter_values(parameters);
  //   autotune::sum.compile();
  // }

  {
    autotune::triad.set_verbose(true);
    auto &builder = autotune::triad.get_builder<cppjit::builder::gcc>();
    builder.set_cpp_flags(
        "-Wall -Wextra -fopenmp -std=c++17 -O3 -funsafe-math-optimizations -g "
        "-march=native -mtune=native -fstrict-aliasing -Iinclude");
    builder.set_include_paths("-Ilikwid/src/includes -Iboost_install/include "
                              "-IVc_install/include -Iinclude");
    builder.set_link_flags("-std=c++17 -O3 -g -fopenmp -fstrict-aliasing ");
    builder.set_library_paths(
        "-Lboost_install/lib -Llikwid -Llikwid/ext/hwloc -Llikwid/ext/lua");
    builder.set_libraries(
        "-lboost_thread -llikwid -llikwid-lua -llikwid-hwloc");
    builder.set_do_cleanup(false);
    autotune::triad.set_parameter_values(parameters);
    autotune::triad.compile();
  }

  size_t N_per_task = 200000000 / 8;
  // adjust to next number divisible by 64
  if (N_per_task % 64 != 0) {
    N_per_task += 64 - (N_per_task % 64);
  }
  // size_t repeat = 200;
  std::cout << "warning: not using non-temporal stores" << std::endl;
  // autotune::copy(N_per_task, repeat);
  // autotune::scale(N_per_task, repeat);
  // autotune::sum(N_per_task, repeat);
  // autotune::triad(N_per_task, repeat);

  double duration_kernel;

  // autotune::copy.set_kernel_duration_functor(
  //     [&duration_kernel]() { return duration_kernel; });
  // autotune::scale.set_kernel_duration_functor(
  //     [&duration_kernel]() { return duration_kernel; });
  // autotune::sum.set_kernel_duration_functor(
  //     [&duration_kernel]() { return duration_kernel; });
  autotune::triad.set_kernel_duration_functor(
      [&duration_kernel]() { return duration_kernel; });

  size_t restarts = 3;
  // {
  //   // autotune::tuners::monte_carlo tuner(autotune::copy, parameters, 20);
  //   // tuner.tune(N_per_task, repeat);
  //   autotune::tuners::line_search tuner_line(autotune::copy, parameters, 20);
  //   tuner_line.set_write_measurement("stream_thread_pool_copy_line");
  //   for (size_t restart = 0; restart < restarts; restart += 1) {
  //     // autotune::countable_set optimal_parameters_line =
  //     tuner_line.tune(N_per_task, repeat, duration_kernel);
  //   }
  //   autotune::tuners::neighborhood_search tuner_neighbor(autotune::copy,
  //                                                        parameters, 10);
  //   tuner_neighbor.set_write_measurement("stream_thread_pool_copy_neighbor");
  //   for (size_t restart = 0; restart < restarts; restart += 1) {
  //     // autotune::countable_set optimal_parameters_neighbor =
  //     tuner_neighbor.tune(N_per_task, repeat, duration_kernel);
  //   }
  // }

  // {
  //   // autotune::tuners::monte_carlo tuner(autotune::copy, parameters, 20);
  //   // tuner.tune(N_per_task, repeat);
  //   autotune::tuners::line_search tuner_line(autotune::scale, parameters,
  //   20); tuner_line.set_write_measurement("stream_thread_pool_scale_line");
  //   for (size_t restart = 0; restart < restarts; restart += 1) {
  //     // autotune::countable_set optimal_parameters_line =
  //     tuner_line.tune(N_per_task, repeat, duration_kernel);
  //   }
  //   autotune::tuners::neighborhood_search tuner_neighbor(autotune::scale,
  //                                                        parameters, 10);
  //   tuner_neighbor.set_write_measurement("stream_thread_pool_scale_neighbor");
  //   for (size_t restart = 0; restart < restarts; restart += 1) {
  //     // autotune::countable_set optimal_parameters_neighbor =
  //     tuner_neighbor.tune(N_per_task, repeat, duration_kernel);
  //   }
  // }

  // {
  //   // autotune::tuners::monte_carlo tuner(autotune::copy, parameters, 20);
  //   // tuner.tune(N_per_task, repeat);
  //   autotune::tuners::line_search tuner_line(autotune::sum, parameters, 20);
  //   tuner_line.set_write_measurement("stream_thread_pool_sum_line");
  //   for (size_t restart = 0; restart < restarts; restart += 1) {
  //     // autotune::countable_set optimal_parameters_line =
  //     tuner_line.tune(N_per_task, repeat, duration_kernel);
  //   }
  //   autotune::tuners::neighborhood_search tuner_neighbor(autotune::sum,
  //                                                        parameters, 10);
  //   tuner_neighbor.set_write_measurement("stream_thread_pool_sum_neighbor");
  //   for (size_t restart = 0; restart < restarts; restart += 1) {
  //     // autotune::countable_set optimal_parameters_neighbor =
  //     tuner_neighbor.tune(N_per_task, repeat, duration_kernel);
  //   }
  // }

  size_t giga_bytes_min = 200; // kernel also requries at least 10 reps
  {
    // autotune::tuners::monte_carlo tuner(autotune::copy, parameters, 20);
    // tuner.tune(N_per_task, repeat);
    // autotune::tuners::parallel_line_search tuner_line(autotune::triad,
    //                                                   parameters, 50);
    // tuner_line.set_write_measurement("stream_thread_pool_triad_line");
    // for (size_t restart = 0; restart < restarts; restart += 1) {
    //   // autotune::countable_set optimal_parameters_line =
    //   for (size_t i = 0; i < parameters.size(); i += 1) {
    //     parameters[i]->set_random_value();
    //   }
    //   tuner_line.tune(N_per_task, giga_bytes_min, duration_kernel);
    // }
    autotune::tuners::parallel_neighborhood_search tuner_neighbor(
        autotune::triad, parameters, 10);
    tuner_neighbor.set_write_measurement("stream_thread_pool_triad_neighbor");
    // autotune::countable_set optimal_parameters_neighbor =
    for (size_t restart = 0; restart < restarts; restart += 1) {
      for (size_t i = 0; i < parameters.size(); i += 1) {
        parameters[i]->set_random_value();
      }
      tuner_neighbor.tune(N_per_task, giga_bytes_min, duration_kernel);
    }
  }

  return 0;
}
