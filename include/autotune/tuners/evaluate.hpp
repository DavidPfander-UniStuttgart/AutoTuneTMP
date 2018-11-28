#pragma once

namespace autotune {

enum class evaluate_t { better, worse_equal, skipped_failed };

namespace detail {

// needs to be instantiated explictly, deduction won't work
// reason: Args will be deduced without cs-quals in the last arguments, but
// might require cv-quals in abstract_tuner-template or tuner_t-template

// returns whether evaluate lead to new optimal configuration found
template <template <typename, typename, typename...> typename tuner_t,
          typename parameter_interface, typename R, typename... Args>
evaluate_t
evaluate_parameters(tuner_t<parameter_interface, R, Args...> &tuner,
                    abstract_kernel<R, cppjit::detail::pack<Args...>> &kernel,
                    parameter_interface &adjusted_parameters, Args &... args) {
  bool verbose = tuner.is_verbose();

  parameter_value_set adjusted_parameter_values =
      to_parameter_values(adjusted_parameters);

  if (verbose) {
    std::cout << "------ begin eval ------" << std::endl;
    print_parameter_values(adjusted_parameter_values);
  }

  double duration_compile = 0.0;
  if (!kernel.is_compiled()) {
    auto start_compile = std::chrono::high_resolution_clock::now();
    kernel.compile();
    auto end_compile = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_compile_duration =
        end_compile - start_compile;
    duration_compile = duration_compile_duration.count();
  }

  // perform post-compile check if registered
  if (!kernel.is_valid_parameter_combination()) {
    if (verbose) {
      std::cout << "invalid parameter combination encountered" << std::endl;
    }
    return evaluate_t::skipped_failed;
  } else {
    if (verbose) {
      std::cout << "parameter combination is valid" << std::endl;
    }
  }

  auto start = std::chrono::high_resolution_clock::now();

  // call kernel, discard possibly returned values
  if constexpr (!std::is_same<R, void>::value) {
    if (tuner.has_test()) {
      for (size_t i = 0; i < tuner.get_repetitions(); i++) {
        bool test_ok = tuner.test(kernel(args...));
        if (!test_ok) {
          if (verbose) {
            std::cout << "warning: test for combination failed!" << std::endl;
          }
          return evaluate_t::skipped_failed;
        } else {
          if (verbose) {
            std::cout << "test for combination passed" << std::endl;
          }
        }
      }
    } else {
      for (size_t i = 0; i < tuner.get_repetitions(); i++) {
        kernel(args...);
      }
    }
  } else {
    for (size_t i = 0; i < tuner.get_repetitions(); i++) {
      kernel(args...);
    }
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration = end - start;
  size_t repetitions = tuner.get_repetitions();
  if (verbose) {
    if (kernel.has_kernel_duration_functor()) {
      double internal_duration = kernel.get_internal_kernel_duration();
      std::cout << "internal duration: " << internal_duration << std::endl;
      if (repetitions > 1) {
        std::cout << "internal duration per repetition: "
                  << (internal_duration / static_cast<double>(repetitions))
                  << std::endl;
      }
      std::cout << "(duration tuner: " << duration.count() << "s)" << std::endl;
      if (repetitions > 1) {
        std::cout << "(duration tuner per repetition: "
                  << (duration.count() / static_cast<double>(repetitions))
                  << "s)" << std::endl;
      }
    } else {
      std::cout << "duration: " << duration.count() << "s" << std::endl;
      if (repetitions > 1) {
        std::cout << "duration tuner per reptition: "
                  << (duration.count() / static_cast<double>(repetitions))
                  << "s" << std::endl;
      }
      std::cout << "------- end eval -------" << std::endl;
    }
  }

  double final_duration;
  if (kernel.has_kernel_duration_functor()) {
    final_duration = kernel.get_internal_kernel_duration();
  } else {
    final_duration = duration.count();
  }
  return tuner.update_parameters(adjusted_parameters, final_duration,
                                 duration_compile);
}

} // namespace detail
} // namespace autotune
