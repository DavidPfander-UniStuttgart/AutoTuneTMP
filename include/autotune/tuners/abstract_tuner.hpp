#pragma once

#include "../abstract_kernel.hpp"
#include "../constraint_graph.hpp"
#include "../simple_constraints.hpp"
#include "parameter_result_cache.hpp"
#include "with_tests.hpp"

#include <chrono>

namespace autotune {

template <typename parameter_interface, typename R, typename... Args>
class abstract_tuner
    : public std::conditional<!std::is_same<R, void>::value,
                              with_tests<R, Args...>,
                              without_tests<R, Args...>>::type {
protected:
  autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f;
  parameter_interface parameters;         // TODO: copy?
  parameter_interface optimal_parameters; // adjusted
  parameter_value_set optimal_parameter_values;
  double optimal_duration;
  bool verbose;
  bool do_measurement;
  bool do_write_header;
  std::ofstream scenario_kernel_duration_file;
  std::ofstream scenario_compile_duration_file;

  parameter_result_cache result_cache;

  std::function<void(parameter_interface &)> parameter_adjustment_functor;
  std::function<void(parameter_value_set &)>
      parameter_values_adjustment_functor;

  std::shared_ptr<simple_constraints> simple_constraints_wrapper;
  std::shared_ptr<constraint_graph> constraint_graph_wrapper;
  // std::function<void(constraint &)> constraints_functor;

  size_t repetitions;

  bool clear_tuner;

  virtual void tune_impl(Args &... args) = 0;

public:
  abstract_tuner(autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
                 parameter_interface &parameters)
      : f(f), parameters(parameters), optimal_duration(-1.0), verbose(false),
        do_measurement(false), do_write_header(true), repetitions(1),
        clear_tuner(true) {}

  parameter_interface tune(Args &... args) {
    // if first run or auto clear is active
    if (clear_tuner || optimal_duration < 0) {
      result_cache.clear();
      optimal_duration = -1.0;
      optimal_parameters = parameters;
      optimal_parameter_values = this->f.get_parameter_values();
    }

    parameter_value_set original_values = this->f.get_parameter_values();

    tune_impl(args...);
    this->f.set_parameter_values(original_values);

    // TODO: API change to parameters hold meta-information only, and not values
    if (this->parameter_adjustment_functor) {
      this->parameter_adjustment_functor(optimal_parameters);
    }
    if (parameter_values_adjustment_functor) {
      // TODO: HACK! FIX after deadline! reason see above
      if (optimal_parameter_values.size() == optimal_parameters.size()) {
        throw;
      }
    }
    return optimal_parameters;
  }

  // returns whether evaluate lead to new optimal configuration found
  bool evaluate(Args &... args) {
    // save original paramters
    parameter_value_set parameter_values = f.get_parameter_values();
    for (size_t parameter_index = 0; parameter_index < parameters.size();
         parameter_index++) {
      auto &p = parameters[parameter_index];
      parameter_values[p->get_name()] = p->get_value();
    }
    // create a copy, so that adjustment can be done
    // (without changing the configured parameters)
    parameter_interface evaluate_parameters = parameters;

    // adjust parameters by parameter set
    if (parameter_adjustment_functor) {
      if (verbose) {
        std::cout << "------ parameters pre-adjustment ------" << std::endl;
        evaluate_parameters.print_values();
        std::cout << "--------------------------" << std::endl;
      }
      if (parameter_adjustment_functor) {
        parameter_adjustment_functor(evaluate_parameters);
        for (size_t parameter_index = 0;
             parameter_index < evaluate_parameters.size(); parameter_index++) {
          auto &p = evaluate_parameters[parameter_index];
          parameter_values[p->get_name()] = p->get_value();
        }
      }
    }
    // adjust parameters by values
    if (parameter_values_adjustment_functor) {
      if (verbose) {
        std::cout << "------ parameters pre-adjustment (value functor) ------"
                  << std::endl;
        // parameters.print_values();
        print_parameter_values(parameter_values);
        std::cout << "--------------------------" << std::endl;
      }
      if (parameter_values_adjustment_functor) {
        parameter_values_adjustment_functor(parameter_values);
      }
    }

    if (verbose) {
      std::cout << "------ post-adjustment values ------" << std::endl;
      // parameters.print_values();
      print_parameter_values(parameter_values);
      std::cout << "--------------------------" << std::endl;
    }

    // parameter_value_set parameter_values = f.get_parameter_values();
    if (!result_cache.contains(parameter_values)) {
      if (verbose) {
        std::cout << "------ add to cache ------" << std::endl;
        print_parameter_values(parameter_values);
      }
      result_cache.insert(parameter_values);
    } else {
      // did_eval = false;

      if (verbose) {
        std::cout << "------ skipped eval ------" << std::endl;
        evaluate_parameters.print_values();
        std::cout << "--------------------------" << std::endl;
      }
      // return std::numeric_limits<double>::max();
      return false;
    }

    if (!f.precompile_validate_parameters(parameter_values)) {
      if (verbose) {
        std::cout << "------ invalidated eval (precompile) ------" << std::endl;
        evaluate_parameters.print_values();
        std::cout << "--------------------------" << std::endl;
      }
      // did_eval = false;
      // return std::numeric_limits<double>::max();
      return false;
    } else {
      if (verbose) {
        std::cout << "parameter combination passed precompile check"
                  << std::endl;
      }
    }

    parameter_value_set original_kernel_parameters = f.get_parameter_values();
    f.set_parameter_values(parameter_values);

    if (do_measurement && do_write_header) {
      this->write_header();
      do_write_header = false;
    }

    // did_eval = true;

    if (verbose) {
      std::cout << "------ begin eval ------" << std::endl;
      // parameters.print_values();
      print_parameter_values(parameter_values);
    }

    f.create_parameter_file();

    auto start_compile = std::chrono::high_resolution_clock::now();
    f.compile();
    auto end_compile = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_compile =
        end_compile - start_compile;

    if (!f.is_valid_parameter_combination()) {
      if (verbose) {
        std::cout << "invalid parameter combination encountered" << std::endl;
      }
      // did_eval = false;
      // return std::numeric_limits<double>::max();
      f.set_parameter_values(original_kernel_parameters);
      return false;
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
          for (size_t i = 0; i < repetitions; i++) {
            bool test_ok = this->test(f(args...));
            if (!test_ok) {
              if (verbose) {
                std::cout << "warning: test for combination failed!"
                          << std::endl;
              }
              // return std::numeric_limits<double>::max();
              f.set_parameter_values(original_kernel_parameters);
              return false;
            } else {
              if (verbose) {
                std::cout << "test for combination passed" << std::endl;
              }
            }
          }
        } else {
          for (size_t i = 0; i < repetitions; i++) {
            f(args...);
          }
        }
      }
    else {
      for (size_t i = 0; i < repetitions; i++) {
        f(args...);
      }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    if (verbose) {
      if (f.has_kernel_duration_functor()) {
        std::cout << "internal duration: " << f.get_internal_kernel_duration()
                  << std::endl;
        if (repetitions > 1) {
          std::cout << "internal duration per repetition: "
                    << (f.get_internal_kernel_duration() /
                        static_cast<double>(repetitions))
                    << std::endl;
        }
        std::cout << "(duration tuner: " << duration.count() << "s)"
                  << std::endl;
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
    if (f.has_kernel_duration_functor()) {
      if (do_measurement) {
        this->write_measurement(f.get_internal_kernel_duration(),
                                duration_compile.count());
      }
      final_duration = f.get_internal_kernel_duration();
    } else {
      if (do_measurement) {
        this->write_measurement(duration.count(), duration_compile.count());
      }
      final_duration = duration.count();
    }
    bool is_better = false;
    if (optimal_duration < 0.0 || final_duration < optimal_duration) {
      optimal_duration = final_duration;
      optimal_parameter_values = parameter_values;
      optimal_parameters = evaluate_parameters;
      this->report_verbose("new best kernel", optimal_duration,
                           evaluate_parameters);
      is_better = true;
    }

    f.set_parameter_values(original_kernel_parameters);
    return is_better;
  }

  void set_verbose(bool verbose) { this->verbose = verbose; }

  const parameter_value_set &get_optimal_parameter_values() const {
    if (optimal_duration < 0.0)
      return f.get_parameter_values();
    else
      return optimal_parameter_values;
  }

  void set_write_measurement(const std::string &scenario_name) {
    if (do_measurement) {
      if (scenario_kernel_duration_file.is_open()) {
        scenario_kernel_duration_file.close();
      }
      if (scenario_compile_duration_file.is_open()) {
        scenario_compile_duration_file.close();
      }
    }
    do_measurement = true;
    do_write_header = true;
    scenario_kernel_duration_file.open(scenario_name + "_kernel_duration.csv");
    scenario_compile_duration_file.open(scenario_name +
                                        "_compile_duration.csv");
  }

  void set_parameter_adjustment_functor(
      std::function<void(parameter_interface &)> parameter_adjustment_functor) {
    this->parameter_adjustment_functor = parameter_adjustment_functor;
    this->parameter_values_adjustment_functor = nullptr;
  }

  void set_parameter_values_adjustment_functor(
      std::function<void(parameter_value_set &)>
          parameter_values_adjustment_functor) {
    this->parameter_values_adjustment_functor =
        parameter_values_adjustment_functor;
    this->parameter_adjustment_functor = nullptr;
  }

  // execute kernel multiple times to average across the result
  void set_repetitions(size_t repetitions) { this->repetitions = repetitions; }

  // reset cache and optima for each run;
  void set_auto_clear(bool clear_tuner) { this->clear_tuner = clear_tuner; };

  parameter_interface &get_parameters() { return parameters; }

  void set_simple_constraints(
      autotune::simple_constraints &simple_constraints_wrapper) {
    this->simple_constraints_wrapper = &simple_constraints_wrapper;
  }

  void
  set_constraint_graph(autotune::simple_constraints &constraint_graph_wrapper) {
    this->constraint_graph_wrapper = &constraint_graph_wrapper;
  }

private:
  void report(const std::string &message, double duration,
              parameter_interface &parameters) {
    std::cout << message << "; duration: " << duration << std::endl;
    parameters.print_values();
  }

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
        scenario_kernel_duration_file << ", ";
        scenario_compile_duration_file << ", ";
      } else {
        first = false;
      }
      scenario_kernel_duration_file << p.first;
      scenario_compile_duration_file << p.first;
    }
    scenario_kernel_duration_file << ", "
                                  << "duration" << std::endl;
    scenario_compile_duration_file << ", "
                                   << "duration" << std::endl;
  }

  void write_measurement(double duration_kernel_s, double duration_compile_s) {
    const parameter_value_set &parameter_values = f.get_parameter_values();
    bool first = true;
    for (auto &p : parameter_values) {
      if (!first) {
        scenario_kernel_duration_file << ", ";
        scenario_compile_duration_file << ", ";
      } else {
        first = false;
      }
      scenario_kernel_duration_file << p.second;
      scenario_compile_duration_file << p.second;
    }
    scenario_kernel_duration_file << ", " << duration_kernel_s << std::endl;
    scenario_compile_duration_file << ", " << duration_compile_s << std::endl;
  }
};
} // namespace autotune
