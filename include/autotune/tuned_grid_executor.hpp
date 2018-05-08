#pragma once

#include <chrono>
#include <cinttypes>
#include <cstddef>
#include <functional>

#include "autotune.hpp"
#include "queue_thread_pool.hpp"
#include "tuners/grid_bruteforce.hpp"

namespace autotune {

class grid_spec {
public:
  size_t grid_z;
  size_t grid_y;
  size_t grid_x;

  size_t block_z;
  size_t block_y;
  size_t block_x;
};

template <size_t num_threads, size_t vector_width, typename... Args>
class tuned_grid_executor {
private:
  cppjit_kernel<void, cppjit::detail::pack<Args...>> &kernel;
  grid_spec spec;
  bool in_tuning_phase;

  tuners::grid_bruteforce tuner;

public:
  tuned_grid_executor(
      cppjit_kernel<void, cppjit::detail::pack<Args...>> &kernel,
      grid_spec spec, countable_set parameters)
      : kernel(kernel), spec(spec), in_tuning_phase(true), tuner(parameters) {
    auto &b = this->kernel.template get_builder<cppjit::builder::gcc>();
    std::string includes = b.get_include_paths();
    b.set_include_paths(includes + std::string(" -Igrid_include -Iinclude"));
  }

  void operator()(Args... args) {
    autotune::queue_thread_pool<num_threads> pool;

    std::function<void(grid_spec, thread_meta)> thread_wrapper = [this,
                                                                  &args...](
        grid_spec spec, thread_meta meta_base) {

      cppjit_kernel<void, cppjit::detail::pack<Args...>> kernel_clone(kernel);

      if (!kernel_clone.is_compiled()) {
        kernel_clone.compile();
      }
      void *uncasted_function = kernel_clone.load_other_symbol("set_meta");
      void (*set_meta_pointer)(thread_meta) =
          reinterpret_cast<decltype(set_meta_pointer)>(uncasted_function);

      countable_set cur_parameters;
      bool found = false;
      if (in_tuning_phase) {
        cur_parameters = tuner.get_next(found);
        if (found) {
          kernel_clone.set_parameter_values(cur_parameters);
        }
      }
      if (!found) {
        cur_parameters = tuner.get_best();
        kernel_clone.set_parameter_values(cur_parameters);
      }

      std::chrono::high_resolution_clock::time_point start_stamp =
          std::chrono::high_resolution_clock::now();

      for (size_t block_z = 0; block_z < spec.block_z; block_z++) {
        for (size_t block_y = 0; block_y < spec.block_y; block_y++) {
          for (size_t block_x = 0; block_x < spec.block_x;
               block_x += vector_width) {
            thread_meta meta = meta_base;
            meta.z += block_z;
            meta.y += block_y;
            meta.x += block_x;

            set_meta_pointer(meta);

            kernel_clone(args...);
          }
        }
      }
      std::chrono::high_resolution_clock::time_point stop_stamp =
          std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> time_span =
          std::chrono::duration_cast<std::chrono::duration<double>>(
              stop_stamp - start_stamp);
      if (in_tuning_phase) {
        double duration = time_span.count();
        tuner.update_best(cur_parameters, duration);
      }

    };

    pool.start();
    for (size_t grid_z = 0; grid_z < spec.grid_z; grid_z++) {
      for (size_t grid_y = 0; grid_y < spec.grid_y; grid_y++) {
        for (size_t grid_x = 0; grid_x < spec.grid_x; grid_x++) {
          thread_meta meta_base;
          meta_base.z = grid_z * spec.block_z;
          meta_base.y = grid_y * spec.block_y;
          meta_base.x = grid_x * spec.block_x;

          pool.enqueue_work(thread_wrapper, spec, meta_base);
        }
      }
    }
    pool.finish();
    std::cout << "pool finished!" << std::endl;
  }
};
}
