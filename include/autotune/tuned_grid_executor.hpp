#pragma once

#include <chrono>
#include <cinttypes>
#include <cstddef>
#include <functional>
#include <memory>

#include "abstract_kernel.hpp"
#include "autotune.hpp"
#include "generalized_kernel.hpp"
#include "queue_thread_pool.hpp"
#include "tuners/grid_bruteforce.hpp"

namespace autotune {

thread_meta get_meta();

void set_meta(thread_meta meta);

class grid_spec {
public:
  size_t grid_z;
  size_t grid_y;
  size_t grid_x;

  size_t block_z;
  size_t block_y;
  size_t block_x;
};

// template <size_t num_threads, size_t vector_width,
//           template <class R, template <class... Args> class pack_type> class
//           kernel_type>;
template <size_t num_threads, size_t vector_width,
          template <class, class> class kernel_type, typename... Args>
class tuned_grid_executor {
private:
  // kernel_type<void, cppjit::detail::pack<Args...>> &kernel;
  kernel_type<void, cppjit::detail::pack<Args...>> &kernel;
  grid_spec spec;
  bool in_tuning_phase;

  tuners::grid_bruteforce tuner;

public:
  tuned_grid_executor(kernel_type<void, cppjit::detail::pack<Args...>> &kernel,
                      grid_spec spec, countable_set parameters)
      : kernel(kernel), spec(spec), in_tuning_phase(true), tuner(parameters) {
    if (cppjit_kernel<void, cppjit::detail::pack<Args...>> *casted =
            dynamic_cast<cppjit_kernel<void, cppjit::detail::pack<Args...>> *>(
                &kernel)) {
      auto &b = casted->template get_builder<cppjit::builder::gcc>();
      std::string includes = b.get_include_paths();
      b.set_include_paths(includes + std::string(" -Igrid_include -Iinclude"));
    }
  }

  void operator()(Args... args) {
    autotune::queue_thread_pool<num_threads> pool;

    std::function<void(grid_spec, thread_meta)> thread_wrapper = [this,
                                                                  &args...](
        grid_spec spec, thread_meta meta_base) {

      std::cout << "starting thread" << std::endl;
      std::shared_ptr<kernel_type<void, cppjit::detail::pack<Args...>>>
      kernel_clone(
          dynamic_cast<kernel_type<void, cppjit::detail::pack<Args...>> *>(
              kernel.clone()));

      if (!kernel_clone->is_compiled()) {
        kernel_clone->compile();
      }
      // void (*set_meta_pointer)(thread_meta) = nullptr;
      // if (std::shared_ptr<cppjit_kernel<void, cppjit::detail::pack<Args...>>>
      // casted =
      //         std::dynamic_pointer_cast<cppjit_kernel<void,
      //         cppjit::detail::pack<Args...>>>(
      //             kernel_clone)) {
      //   void *uncasted_function = casted->load_other_symbol("set_meta");
      //   set_meta_pointer =
      //   reinterpret_cast<decltype(set_meta_pointer)>(uncasted_function);
      // }
      // auto generalized_kernel_ptr =
      //     std::dynamic_pointer_cast<generalized_kernel<void,
      //     cppjit::detail::pack<Args...>>>(
      //         kernel_clone);

      countable_set cur_parameters;
      bool found = false;
      if (in_tuning_phase) {
        cur_parameters = tuner.get_next(found);
        if (found) {
          kernel_clone->set_parameter_values(cur_parameters);
        }
      }
      if (!found) {
        cur_parameters = tuner.get_best();
        kernel_clone->set_parameter_values(cur_parameters);
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

            if
              constexpr(
                  std::is_same<kernel_type<void, cppjit::detail::pack<Args...>>,
                               generalized_kernel<void, cppjit::detail::pack<
                                                            Args...>>>::value) {
                std::cout << "before meta.z: " << meta.z << std::endl;
                std::cout << "before meta.y: " << meta.y << std::endl;
                std::cout << "before meta.x: " << meta.x << std::endl;
                set_meta(meta);
                (*kernel_clone)(args...);
              }
            else {
              std::cout << "before meta.z: " << meta.z << std::endl;
              std::cout << "before meta.y: " << meta.y << std::endl;
              std::cout << "before meta.x: " << meta.x << std::endl;
              kernel_clone->set_meta(meta);
              (*kernel_clone)(args...);
            }

            // if (generalized_kernel_ptr) {
            //   set_meta(meta);
            //   (*kernel_clone)(args...);
            // } else {
            //   kernel_clone->set_meta(meta);
            //   (*kernel_clone)(args...);
            // }
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
} // namespace autotune
// namespace autotune
