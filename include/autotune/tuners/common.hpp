#pragma once

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
public:
  abstract_tuner() = default;

  double evaluate(bool &is_valid, parameter_interface &parameters,
                  autotune::kernel<R, cppjit::detail::pack<Args...>> &f,
                  Args &... args) {

    f.set_parameter_values(parameters);

    is_valid = true;

    if (f.is_verbose()) {
      std::cout << "------ begin eval ------" << std::endl;
      f.print_values();
    }

    f.create_parameter_file();

    f.compile();

    if (!f.is_valid_parameter_combination()) {
      if (f.is_verbose()) {
        std::cout << "invalid parameter combination encountered" << std::endl;
      }
      is_valid = false;
      return std::numeric_limits<double>::max();
    } else {
      if (f.is_verbose()) {
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
            if (f.is_verbose()) {
              std::cout << "warning: test for combination failed!" << std::endl;
            }
            return std::numeric_limits<double>::max();
          } else {
            if (f.is_verbose()) {
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

    if (f.is_verbose()) {
      std::cout << "duration: " << duration.count() << "s" << std::endl;
      std::cout << "------- end eval -------" << std::endl;
    }

    f.write_measurement(duration.count());

    return duration.count();
  }

  void report(const std::string &message, double duration,
              typename autotune::kernel<R, cppjit::detail::pack<Args...>> &f) {
    std::cout << message << "; duration: " << duration;
    f.print_values();
  }

  // template <class F>
  void report_verbose(
      const std::string &message, double duration,
      typename autotune::kernel<R, cppjit::detail::pack<Args...>> &f) {
    if (f.is_verbose()) {
      report(message, duration, f);
    }
  }
};
} // namespace autotune
