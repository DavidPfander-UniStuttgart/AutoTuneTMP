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

#include "helpers/timing.hpp"

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

template <size_t num_threads, size_t vector_width,
          template <class, class> class kernel_type, typename... Args>
class tuned_grid_executor {
private:
  kernel_type<void, cppjit::detail::pack<Args...>> &kernel;

  std::array<std::shared_ptr<kernel_type<void, cppjit::detail::pack<Args...>>>,
             num_threads>
      all_kernel_clones;
  std::array<countable_set, num_threads> all_cur_parameters;

  static constexpr size_t max_measurements = 20;
  // next two parameters determine the magnitude of the improvement that can be
  // determined semi-reliably
  static constexpr size_t min_measurements = 4;
  static constexpr double accuracy_threshold = 0.05;
  std::array<size_t, num_threads> all_num_measurements;
  std::array<std::array<double, max_measurements>, num_threads>
      all_measurements;

  grid_spec spec;
  volatile bool in_tuning_phase;
  std::array<bool, num_threads> improve_accuracy;

  tuners::grid_line_search tuner;

  volatile bool final_kernel_compiled;
  bool verbose;

  std::mutex final_compile_mutex;

  double run_block(kernel_type<void, cppjit::detail::pack<Args...>> &kernel,
                   int64_t thread_id, thread_meta &meta_base, Args &... args) {
    std::chrono::high_resolution_clock::time_point start_stamp =
        std::chrono::high_resolution_clock::now();
    uint64_t rdtscp_start = rdtscp_base_cycles_start();
#ifdef USERMODE_RDPMC_ENABLED
    uint64_t rdpmc_start = rdpmc_actual_cycles();
#endif

    for (size_t block_z = 0; block_z < spec.block_z; block_z++) {
      for (size_t block_y = 0; block_y < spec.block_y; block_y++) {
        for (size_t block_x = 0; block_x < spec.block_x;
             block_x += vector_width) {
          thread_meta meta = meta_base;
          meta.z += block_z;
          meta.y += block_y;
          meta.x += block_x;

          if constexpr (std::is_same<
                            kernel_type<void, cppjit::detail::pack<Args...>>,
                            generalized_kernel<
                                void, cppjit::detail::pack<Args...>>>::value) {
            // TODO: should probably add set_thread_id here
            set_meta(meta);
            kernel(args...);
          } else {
            kernel.set_thread_id(thread_id);
            kernel.set_meta(meta);
            kernel(args...);
          }
        }
      }
    }
    std::chrono::high_resolution_clock::time_point stop_stamp =
        std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span =
        std::chrono::duration_cast<std::chrono::duration<double>>(stop_stamp -
                                                                  start_stamp);
#ifdef USERMODE_RDPMC_ENABLED
    uint64_t rdpmc_end = rdpmc_actual_cycles();
#endif
    uint64_t rdtscp_stop = rdtscp_base_cycles_stop();
    double corrected_duration = 0.0;
    if (verbose) {
      double duration_time = time_span.count();
#ifdef USERMODE_RDPMC_ENABLED
      double duration_act_cycles = static_cast<double>(rdpmc_end - rdpmc_start);
      double act_frequency = duration_act_cycles / duration_time;
      std::cout << "rdpmc freq (cycles/duration): " << act_frequency
                << std::endl;

#endif
      std::cout << "raw duration: " << duration_time << std::endl;
      double base_freq = ((rdtscp_stop - rdtscp_start) / duration_time);
      std::cout << "base frequency (from steady rdtscp): " << base_freq
                << std::endl;
#ifdef USERMODE_RDPMC_ENABLED

      double fraction_freq = act_frequency / base_freq;
      std::cout << "fraction_freq: " << fraction_freq << std::endl;
      corrected_duration = fraction_freq * duration_time;
      std::cout << "frequency-corrected duration: " << corrected_duration
                << std::endl;
#endif
    }

#ifdef USERMODE_RDPMC_ENABLED
    return corrected_duration;
#else
    return time_span.count();
#endif
  }

  void compute_statistics(std::array<double, max_measurements> &measurements,
                          size_t index_start, size_t index_stop, double &min,
                          double &max, double &average) {
    min = 0.0;
    max = 0.0;
    double sum = 0.0;
    bool first = true;

    for (size_t i = index_start; i < index_stop; i++) {
      if (first) {
        min = measurements[i];
        max = measurements[i];
        first = false;
      }
      if (measurements[i] < min) {
        min = measurements[i];
      }
      if (measurements[i] > min) {
        max = measurements[i];
      }
      sum += measurements[i];
    }
    average = sum / static_cast<double>(index_stop - index_start);
  }

  double evaluate_accuracy(int64_t thread_id, double duration,
                           bool &measurement_accurate) {
    size_t &num_measurements = all_num_measurements[thread_id];
    std::array<double, max_measurements> &measurements =
        all_measurements[thread_id];

    // add the current measurement (requires max_measurements > 0)
    measurements[num_measurements] = duration;
    num_measurements += 1;

    double min;
    double max;
    double average;

    compute_statistics(measurements, 0, num_measurements, min, max, average);

    // reached maximum evaluations, return average
    if (num_measurements >= max_measurements) {
      std::cout << "tuned_grid_executor: evaluations not stable after "
                   "num_measurements: "
                << num_measurements << " min: " << min
                << " average: " << average << " max: " << max << std::endl;
      measurement_accurate = true;
      return average;
    }

    // do at least 3 measurements
    if (num_measurements < min_measurements) {
      measurement_accurate = false;
      return average;
    }

    compute_statistics(measurements, num_measurements - min_measurements,
                       num_measurements, min, max, average);

    // if (max == average) {
    //   measurement_accurate = true;
    //   return max;
    // }

    // ~x% surrounding fence
    double max_fraction = average / max;
    double min_fraction = average / min;
    if (max_fraction > (1.0 - accuracy_threshold) &&
        min_fraction < (1.0 + accuracy_threshold)) {
      if (verbose) {
        std::cout << "tuned_grid_executor: measurement accepted after "
                     "num_measurements: "
                  << num_measurements << " min: " << min
                  << " average: " << average << " max: " << max << std::endl;
      }
      measurement_accurate = true;
      return min;
    }

    if (verbose) {
      std::cout << "tuned_grid_executor: rejected due to accuracy after "
                   "num_measurements: "
                << num_measurements << " min: " << min
                << " average: " << average << " max: " << max << std::endl;
    }

    measurement_accurate = false;
    return average;
  }

public:
  tuned_grid_executor(kernel_type<void, cppjit::detail::pack<Args...>> &kernel,
                      grid_spec spec, countable_set parameters)
      : kernel(kernel), spec(spec), in_tuning_phase(true),
        tuner(parameters, 1, true), final_kernel_compiled(false),
        verbose(true) {

    std::fill(improve_accuracy.begin(), improve_accuracy.end(), false);

    if (cppjit_kernel<void, cppjit::detail::pack<Args...>> *casted =
            dynamic_cast<cppjit_kernel<void, cppjit::detail::pack<Args...>> *>(
                &kernel)) {
      auto &b = casted->template get_builder<cppjit::builder::gcc>();
      std::string includes = b.get_include_paths();
      b.set_include_paths(includes + std::string(" -Igrid_include -Iinclude"));
    }
  }

  void operator()(Args... args) {
    autotune::queue_thread_pool<num_threads> pool(verbose);

    std::function<void(int64_t, grid_spec, thread_meta)> thread_wrapper =
        [this, &args...](int64_t thread_id, grid_spec spec,
                         thread_meta meta_base) {
          if (in_tuning_phase) {

            auto &kernel_clone = all_kernel_clones[thread_id];

            if (!kernel_clone) {
              std::cout << "tuned_grid_executor: cloning kernel, thread_id: "
                        << thread_id << std::endl;
              kernel_clone = std::shared_ptr<
                  kernel_type<void, cppjit::detail::pack<Args...>>>(
                  dynamic_cast<kernel_type<void, cppjit::detail::pack<Args...>>
                                   *>(kernel.clone()));
            }

            if (!improve_accuracy[thread_id]) {
              countable_set cur_parameters;
              bool still_tuning = false;
              bool update = false;
              cur_parameters = tuner.get_next(still_tuning, update);
              if (still_tuning) {
                kernel_clone->set_parameter_values(cur_parameters);
                kernel_clone->compile();

                if (update) {
                  all_cur_parameters[thread_id] = cur_parameters;
                  all_num_measurements[thread_id] = 0;
                  improve_accuracy[thread_id] = true;
                } else {
                  // double duration =
                  run_block(*kernel_clone, thread_id, meta_base, args...);
                  // otherwise the !in_tuning_phase can be entered (data race)
                  return;
                }
              } else {
                in_tuning_phase = false;
              }
            }

            if (improve_accuracy[thread_id]) { // improve_accuracy implies
                                               // update
              double duration =
                  run_block(*kernel_clone, thread_id, meta_base, args...);
              bool measurement_accurate = false;
              double duration_report =
                  evaluate_accuracy(thread_id, duration, measurement_accurate);
              if (measurement_accurate) {
                tuner.update_best(all_cur_parameters[thread_id],
                                  duration_report);
                improve_accuracy[thread_id] = false;
              }
              // otherwise the !in_tuning_phase can be entered (data race)
              return;
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
}; // namespace autotune
} // namespace autotune
// namespace autotune
