#pragma once

#include "../abstract_kernel.hpp"

#include <chrono>

namespace autotune {

template <typename R, typename... Args> class with_tests {
private:
  std::function<bool(R)> t;

public:
  void setup_test(std::function<bool(R)> t_) { t = t_; };

  bool has_test() { return t ? true : false; }

  bool test(R r) { return t(r); };
};

template <typename R, typename... Args> class without_tests {};

template <typename parameter_interface, typename R, typename... Args>
class abstract_tuner
    : public std::conditional<!std::is_same<R, void>::value,
                              with_tests<R, Args...>,
                              without_tests<R, Args...>>::type {
protected:
  bool verbose;

public:
  abstract_tuner() : verbose(false) {}

  double
  evaluate(bool &is_valid, parameter_interface &parameters,
           autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
           Args &... args) {

    f.set_parameter_values(parameters);

    is_valid = true;

    if (verbose) {
      std::cout << "------ begin eval ------" << std::endl;
      parameters.print_values();
    }

    f.create_parameter_file();

    f.compile();

    if (!f.is_valid_parameter_combination()) {
      if (verbose) {
        std::cout << "invalid parameter combination encountered" << std::endl;
      }
      is_valid = false;
      return std::numeric_limits<double>::max();
    } else {
      if (verbose) {
        std::cout << "parameter combination is valid" << std::endl;
      }
    }

    auto start = std::chrono::high_resolution_clock::now();

    // call kernel, discard possibly returned values
    if
      constexpr(!std::is_same<R, void>::value) {
        if (this->has_test()) {
          bool test_ok = this->test(f(args...));
          if (!test_ok) {
            if (verbose) {
              std::cout << "warning: test for combination failed!" << std::endl;
            }
            return std::numeric_limits<double>::max();
          } else {
            if (verbose) {
              std::cout << "test for combination passed" << std::endl;
            }
          }
        } else {
          f(args...);
        }
      }
    else {
      f(args...);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    if (verbose) {
      std::cout << "duration: " << duration.count() << "s" << std::endl;
      std::cout << "------- end eval -------" << std::endl;
      if (f.has_kernel_duration_functor()) {
        std::cout << "internal duration: " << f.get_internal_kernel_duration()
                  << std::endl;
      }
    }

    if (f.has_kernel_duration_functor()) {
      f.write_measurement(f.get_internal_kernel_duration());
      return f.get_internal_kernel_duration();
    } else {
      f.write_measurement(duration.count());
      return duration.count();
    }
  }

  void report(const std::string &message, double duration,
              parameter_interface &parameters) {
    std::cout << message << "; duration: " << duration << std::endl;
    parameters.print_values();
  }

  void set_verbose(bool verbose) { this->verbose = verbose; }

  void report_verbose(const std::string &message, double duration,
                      parameter_interface &parameters) {
    if (verbose) {
      report(message, duration, parameters);
    }
  }
};
} // namespace autotune
