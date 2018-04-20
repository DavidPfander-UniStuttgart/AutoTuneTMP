#pragma once

#include "autotune_exception.hpp"
#include "cppjit/function_traits.hpp"
#include "parameter_value_set.hpp"
#include "util.hpp"
//#include "parameter_set.hpp"

#include <fstream>

namespace autotune {

template <typename R, typename... Args> class abstract_kernel;

template <typename R, typename... Args>
class abstract_kernel<R, cppjit::detail::pack<Args...>> {
protected:
  bool verbose;

  std::string kernel_name;
  parameter_value_set parameter_values;

  std::function<double()> kernel_duration_functor;

  bool parameters_changed;

  std::function<bool(parameter_value_set &)>
      precompile_validate_parameters_functor;

public:
  abstract_kernel(const std::string &kernel_name)
      : verbose(false), kernel_name(kernel_name), parameters_changed(true) {}

  void set_verbose(bool verbose) { this->verbose = verbose; }

  bool is_verbose() { return verbose; }

  virtual bool is_valid_parameter_combination() = 0;

  template <typename T> void set_parameter_value(const T &p) {
    parameters_changed = true;
    parameter_values[p.get_name()] = p.get_value();
  }

  // for arbitrary parameter interfaces and tuned groups of parameters
  template <typename... Ts>
  void set_parameter_values(const Ts &... parameters) {
    parameters_changed = true;
    auto t = std::make_tuple(std::ref(parameters)...);
    detail::iterate_tuple(t, [this](auto &ps) {
      for (size_t i = 0; i < ps.size(); i++) {
        parameter_values[ps[i]->get_name()] = ps[i]->get_value();
      }
    });
  }

  void set_parameter_values(const parameter_value_set &new_parameter_values) {
    parameters_changed = true;
    for (auto &p : new_parameter_values) {
      parameter_values[p.first] = p.second;
    }
  }

  void clear() {
    parameters_changed = true;
    parameter_values.clear();
  }

  const parameter_value_set &get_parameter_values() const {
    return parameter_values;
  }

  virtual R operator()(Args... args) = 0;

  virtual void compile() = 0;

  virtual bool is_compiled() = 0;

  virtual void create_parameter_file() = 0;

  void
  set_kernel_duration_functor(std::function<double()> kernel_duration_functor) {
    this->kernel_duration_functor = kernel_duration_functor;
  }

  bool has_kernel_duration_functor() {
    if (this->kernel_duration_functor) {
      return true;
    } else {
      return false;
    }
  }

  double get_internal_kernel_duration() {
    if (!this->kernel_duration_functor) {
      throw autotune_exception("no kernel duration functor specified");
    }
    return this->kernel_duration_functor();
  }

  void set_precompile_validate_parameter_functor(
      const std::function<bool(parameter_value_set &parameters)>
          &precompile_validate_parameters) {
    this->precompile_validate_parameters_functor =
        precompile_validate_parameters;
  }

  bool precompile_validate_parameters(parameter_value_set &parameters) {
    if (precompile_validate_parameters_functor) {
      return precompile_validate_parameters_functor(parameters);
    }
    return true;
  }
};
}
