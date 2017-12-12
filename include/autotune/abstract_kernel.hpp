#pragma once

#include "parameter_set.hpp"

#include "cppjit/function_traits.hpp"

#include <fstream>

namespace autotune {

template <typename R, typename... Args> class abstract_kernel;

template <typename R, typename... Args>
class abstract_kernel<R, cppjit::detail::pack<Args...>> {
private:
  bool verbose;
  bool measurement_enabled;
  std::ofstream scenario_measurement_file;

  std::string kernel_name;
  parameter_value_set parameter_values;

public:
  abstract_kernel(const std::string &kernel_name)
      : verbose(false), measurement_enabled(false), kernel_name(kernel_name) {}

  void set_verbose(bool verbose) { this->verbose = verbose; }

  void set_write_measurement(const std::string &scenario_name) {
    if (measurement_enabled) {
      if (scenario_measurement_file.is_open()) {
        scenario_measurement_file.close();
      }
    }
    measurement_enabled = true;
    scenario_measurement_file.open(scenario_name + ".csv");
  }

  // to be called from tuner, not directly
  void write_header() {
    if (!measurement_enabled) {
      return;
    }
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

  // to be called from tuner, not directly
  void write_measurement(double duration_s) {
    if (!measurement_enabled) {
      return;
    }
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

  bool is_verbose() { return verbose; }

  virtual bool is_valid_parameter_combination() = 0;

  void set_parameter_values(parameter_value_set &new_parameter_values) {
    parameter_values.clear();
    for (auto &p : new_parameter_values) {
      parameter_values[p.first] = p.second;
    }
  }

  template <typename parameter_set_type>
  void set_parameter_values(parameter_set_type &parameters) {
    parameter_values.clear();
    for (size_t i = 0; i < parameters.size(); i++) {
      auto &p = parameters[i];
      parameter_values[p->get_name()] = p->get_value();
    }
  }

  parameter_value_set get_parameter_values() { return parameter_values; }

  virtual R operator()(Args... args) = 0;

  // // very useful overload for the kernel tuners, so that they don't have to
  // // track source-related arguments
  // virtual void compile(const std::string &source_dir) = 0;

  // virtual void compile_inline(const std::string &source) = 0;

  virtual void compile() = 0;

  virtual bool is_compiled() = 0;

  // TODO: add parameter_set argument?
  virtual void create_parameter_file() = 0;
};
}
