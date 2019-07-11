#pragma once

#include <chrono>

#include "../constraint_graph.hpp"
#include "../simple_constraints.hpp"
#include "../util.hpp"
#include "abstract_tuner.hpp"
#include "with_tests.hpp"

namespace autotune {

// class simple_constraints;

namespace tuners {

template <typename parameter_interface, typename R, typename... Args>
class group_tuner {
  autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f;
  std::vector<
      std::reference_wrapper<abstract_tuner<parameter_interface, R, Args...>>>
      tuners;
  size_t group_repeat;
  bool verbose;

  std::shared_ptr<simple_constraints> simple_constraints_wrapper;
  std::shared_ptr<constraint_graph> constraint_graph_wrapper;
  std::shared_ptr<csv_reporter> reporter;
  bool write_measurement;
  std::string scenario_name;
  size_t tune_counter;

public:
  template <typename... Rs>
  group_tuner(autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
              size_t group_repeat,
              abstract_tuner<parameter_interface, Rs, Args...> &... tuners)
      : f(f), group_repeat(group_repeat), verbose(false),
        write_measurement(false), scenario_name(""), tune_counter(0) {

    static_assert(detail::is_all_same<R, Rs...>::value);

    // collect tuners
    (this->tuners.push_back(tuners), ...);

    // make sure that parameter value caching across "tune" calls is enabled
    for (size_t i = 0; i < this->tuners.size(); i++) {
      this->tuners[i].get().set_auto_clear(false);
    }
  }

  parameter_value_set tune(Args &... args) {

    parameter_value_set original_values = this->f.get_parameter_values();

    // setup parameters in kernel, so that kernel is aware of all parameters
    // during tuning
    // also collect parameters accross all tuners
    parameter_value_set kernel_total_parameters;
    for (size_t i = 0; i < tuners.size(); i++) {
      auto &tuner_parameters = tuners[i].get().get_parameters();
      f.set_parameter_values(tuner_parameters);
      for (size_t i = 0; i < tuner_parameters.size();
           i += 1) { // to_parameter_values(parameters);
        kernel_total_parameters[tuner_parameters[i]->get_name()] =
            tuner_parameters[i]->get_value();
      }
    }

    tune_counter += 1;
    if (write_measurement) {
      reporter = std::make_shared<csv_reporter>(
          scenario_name, kernel_total_parameters, tune_counter);
      for (size_t i = 0; i < tuners.size(); i++) {
        tuners[i].get().set_meta_reporter(reporter);
      }
    }

    for (size_t group_restart = 0; group_restart < group_repeat;
         group_restart++) {
      if (verbose) {
        std::cout << "group_restart: " << group_restart << std::endl;
      }

      for (size_t i = 0; i < tuners.size(); i++) {
        if (verbose) {
          std::cout << "tuner index: " << i << std::endl;
        }
        std::chrono::high_resolution_clock::time_point start =
            std::chrono::high_resolution_clock::now();
        parameter_interface tuner_optimal_parameters =
            tuners[i].get().tune(args...);
        std::chrono::high_resolution_clock::time_point end =
            std::chrono::high_resolution_clock::now();
        double tuning_duration =
            std::chrono::duration<double>(end - start).count();
        if (verbose) {
          std::cout << "tuner duration: " << tuning_duration << std::endl;
        }
        // propagate current optimal parameters to other tuners
        f.set_parameter_values(tuner_optimal_parameters);
      }
    }

    parameter_value_set optimal_parameter_values = f.get_parameter_values();
    if (verbose) {
      std::cout << "optimal parameter values (group tuner):" << std::endl;
      autotune::print_parameter_values(optimal_parameter_values);
    }
    f.set_parameter_values(original_values);
    return optimal_parameter_values;
  }

  void set_verbose(bool verbose) {
    this->verbose = verbose;
    for (size_t i = 0; i < tuners.size(); i++) {
      tuners[i].get().set_verbose(true);
    }
  }

  void setup_test(std::function<bool(R)> test_functional) {
    for (size_t i = 0; i < this->tuners.size(); i++) {
      this->tuners.setup_test(test_functional);
    }
  };

  void set_write_measurement(const std::string &scenario_name) {
    write_measurement = true;
    this->scenario_name = scenario_name;
    reporter.reset();
  }
};
} // namespace tuners
} // namespace autotune
