#pragma once

#include "../abstract_kernel.hpp"
#include "parameter_result_cache.hpp"

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
  autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f;
  parameter_interface parameters;
  bool verbose;
  bool do_measurement;
  bool do_write_header;
  std::ofstream scenario_measurement_file;

  parameter_result_cache<parameter_interface> result_cache;

  std::function<void(parameter_interface &)> parameter_adjustment_functor;

public:
  abstract_tuner(autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
                 parameter_interface &parameters)
      : f(f), parameters(parameters), verbose(false), do_measurement(false),
        do_write_header(true), scenario_measurement_file("") {}

  double evaluate(bool &did_eval, Args &... args) {

    if (!result_cache.contains(parameters)) {
      result_cache.insert(parameters);
    } else {
      did_eval = false;

      if (verbose) {
        std::cout << "------ skipped eval ------" << std::endl;
        parameters.print_values();
        std::cout << "--------------------------" << std::endl;
      }
      return std::numeric_limits<double>::max();
    }

    if (parameter_adjustment_functor) {
      if (verbose) {
        std::cout << "------ parameters pre-adjustment ------" << std::endl;
        parameters.print_values();
        std::cout << "--------------------------" << std::endl;
      }
      parameter_adjustment_functor(parameters);
    }

    parameter_value_set parameter_values = to_parameter_values(parameters);
    if (!f.precompile_validate_parameters(parameter_values)) {
      if (verbose) {
        std::cout << "------ invalidated eval (precompile) ------" << std::endl;
        parameters.print_values();
        std::cout << "--------------------------" << std::endl;
      }
      did_eval = false;
      return std::numeric_limits<double>::max();
    } else {
      if (verbose) {
        std::cout << "parameter combination passed precompile check"
                  << std::endl;
      }
    }

    f.set_parameter_values(parameter_values);

    if (do_measurement && do_write_header) {
      this->write_header();
      do_write_header = false;
    }

    did_eval = true;

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
      did_eval = false;
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
      if (do_measurement) {
        this->write_measurement(f.get_internal_kernel_duration());
      }
      return f.get_internal_kernel_duration();
    } else {
      if (do_measurement) {
        this->write_measurement(duration.count());
      }
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

  void write_header() {
    const parameter_value_set &parameter_values = f.get_parameter_values();
    bool first = true;
    for (auto &p : parameter_values) {
      if (!first) {
        scenario_measurement_file << ", ";
      } else {
        first = false;
      }
      scenario_measurement_file << p.first;
    }
    scenario_measurement_file << ", "
                              << "duration" << std::endl;
  }

  void write_measurement(double duration_s) {
    const parameter_value_set &parameter_values = f.get_parameter_values();
    bool first = true;
    for (auto &p : parameter_values) {
      if (!first) {
        scenario_measurement_file << ", ";
      } else {
        first = false;
      }
      scenario_measurement_file << p.second;
    }
    scenario_measurement_file << ", " << duration_s << std::endl;
  }

  void set_write_measurement(const std::string &scenario_name) {
    if (do_measurement) {
      if (scenario_measurement_file.is_open()) {
        scenario_measurement_file.close();
      }
    }
    do_measurement = true;
    do_write_header = true;
    scenario_measurement_file.open(scenario_name + ".csv");
  }

  void set_parameter_adjustment_functor(
      std::function<void(parameter_interface &)> parameter_adjustment_functor) {
    this->parameter_adjustment_functor = parameter_adjustment_functor;
  }
};
} // namespace autotune
