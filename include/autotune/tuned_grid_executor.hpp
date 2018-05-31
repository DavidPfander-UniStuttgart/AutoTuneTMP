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
// #include "tuners/grid_bruteforce.hpp"
#include "tuners/grid_line_search.hpp"

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
  volatile bool in_tuning_phase;

  tuners::grid_line_search tuner;

  volatile bool final_kernel_compiled;
  bool verbose;

  std::mutex final_compile_mutex;

  // void thread_wrapper(grid_spec spec, thread_meta meta_base, Args &... args)
  // {
  //   if (verbose) {
  //     std::cout << "starting thread" << std::endl;
  //   }

  //   // if (!in_tuning_phase) { // TODO/CONTINUE: compile only once, then
  //   //                         // reuse "kernel"
  //   //     if (!final_kernel_compiled) {
  //   //         wait();
  //   //     }
  //   // }

  //   if (in_tuning_phase) {
  //     std::shared_ptr<kernel_type<void, cppjit::detail::pack<Args...>>>
  //     kernel_clone(
  //         dynamic_cast<kernel_type<void, cppjit::detail::pack<Args...>> *>(
  //             kernel.clone()));

  //     countable_set cur_parameters;
  //     bool found = false;
  //     cur_parameters = tuner.get_next(found);
  //     if (found) {
  //       kernel_clone->set_parameter_values(cur_parameters);
  //     } else {
  //       in_tuning_phase = false;
  //     }

  //     if (found) {
  //       kernel_clone->set_parameter_values(cur_parameters);
  //       kernel_clone->compile();
  //       double duration = run_block(*kernel_clone, meta_base, args...);
  //       tuner.update_best(cur_parameters, duration);
  //     }
  //   }
  //   if (!in_tuning_phase) {
  //     if (!final_kernel_compiled) {
  //       std::unique_lock<std::mutex>(final_compile_mutex);
  //       if (!final_kernel_compiled) {
  //         countable_set best_parameters = tuner.get_best();
  //         kernel->set_parameter_values(best_parameters);
  //         kernel->compile();
  //         final_kernel_compiled = true;
  //       }
  //     }
  //     double duration = run_block(*kernel, meta_base, args...);
  //     if (verbose) {
  //       std::cout << "tuned grid executor: block duration: " << duration
  //                 << std::endl;
  //     }
  //   }
  // }

  double run_block(kernel_type<void, cppjit::detail::pack<Args...>> &kernel,
                   int64_t thread_id, thread_meta &meta_base, Args &... args) {
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
                             generalized_kernel<
                                 void, cppjit::detail::pack<Args...>>>::value) {
              set_meta(meta);
              kernel(args...);
            }
          else {
            kernel.set_thread_id(thread_id);
            // kernel.set_meta(meta, thread_id);
            kernel.set_meta(meta);
            kernel(args...);
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
        std::chrono::duration_cast<std::chrono::duration<double>>(stop_stamp -
                                                                  start_stamp);
    return time_span.count();
  }

public:
  tuned_grid_executor(kernel_type<void, cppjit::detail::pack<Args...>> &kernel,
                      grid_spec spec, countable_set parameters)
      : kernel(kernel), spec(spec), in_tuning_phase(true),
        tuner(parameters, 1, true), final_kernel_compiled(false),
        verbose(true) {
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

    std::function<void(int64_t, grid_spec, thread_meta)> thread_wrapper =
        [this, &args...](int64_t thread_id, grid_spec spec,
                         thread_meta meta_base) {
          if (verbose) {
            std::cout << "starting thread id: " << thread_id << std::endl;
          }

          if (in_tuning_phase) {

            countable_set cur_parameters;
            bool found = false;
            bool update = false;
            cur_parameters = tuner.get_next(found, update);
            if (found) {
              std::shared_ptr<kernel_type<void, cppjit::detail::pack<Args...>>>
              kernel_clone(dynamic_cast<
                           kernel_type<void, cppjit::detail::pack<Args...>> *>(
                  kernel.clone()));
              kernel_clone->set_parameter_values(cur_parameters);
              kernel_clone->compile();
              double duration =
                  run_block(*kernel_clone, thread_id, meta_base, args...);
              if (update) {
                tuner.update_best(cur_parameters, duration);
              }
            } else {
              in_tuning_phase = false;
            }
          }

          if (!in_tuning_phase) {
            if (!final_kernel_compiled) {
              std::unique_lock lock(final_compile_mutex);
              if (!final_kernel_compiled) {
                if (verbose) {
                  std::cout << "tuned grid executor: now compiling final kernel"
                            << std::endl;
                }
                countable_set best_parameters = tuner.get_best();
                kernel.set_parameter_values(best_parameters);
                kernel.compile();
                final_kernel_compiled = true;
              } else {
                if (verbose) {
                  std::cout << "tuned grid executor: compiled by another thread"
                            << std::endl;
                }
              }
            }
            double duration = run_block(kernel, thread_id, meta_base, args...);
            if (verbose) {
              std::cout << "tuned grid executor: block duration: " << duration
                        << std::endl;
            }
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
          // std::cout << "meta_base.x: " << meta_base.x
          //           << " meta_base.y: " << meta_base.y
          //           << " meta_base.z: " << meta_base.z << std::endl;

          pool.enqueue_work_id(thread_wrapper, pool.THREAD_ID_PLACEHOLDER, spec,
                               meta_base);
          // pool.enqueue_work(thread_wrapper, spec, meta_base);

          // std::function<void(size_t, double, double)> dummy =
          //     [](size_t a, double b, double c) {};
          // pool.enqueue_work_id(dummy, static_cast<size_t>(0), 2.0, 3.0);
        }
      }
    }
    pool.finish();
    if (verbose) {
      std::cout << "tuned grid executor: pool finished!" << std::endl;
    }
  }
};
} // namespace autotune
// namespace autotune
