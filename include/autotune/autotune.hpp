#pragma once

#include <fstream>
#include <iostream>
#include <vector>

#include "cppjit/cppjit.hpp"
#include "cppjit/function_traits.hpp"

#include "abstract_kernel.hpp"
#include "abstract_parameter.hpp"
#include "autotune_exception.hpp"
#include "continuous_parameter.hpp"
#include "fixed_set_parameter.hpp"
#include "parameter_set.hpp"

namespace autotune {

// base template for the following specialization
// required to do the pack-matching in the specialization
template <typename... Args> struct cppjit_kernel;

//

template <typename R, typename... Args>
class cppjit_kernel<R, cppjit::detail::pack<Args...>>
    : public abstract_kernel<R, cppjit::detail::pack<Args...>> {
private:
  bool verbose;
  bool measurement_enabled;
  std::ofstream scenario_measurement_file;

  std::string kernel_name;
  cppjit::kernel<R, cppjit::detail::pack<Args...>> &internal_kernel;
  // parameter_set parameters;
  parameter_value_set parameter_values;

public:
  cppjit_kernel(
      const std::string &kernel_name,
      cppjit::kernel<R, cppjit::detail::pack<Args...>> &internal_kernel)
      : abstract_kernel<R, cppjit::detail::pack<Args...>>(kernel_name),
        verbose(false), measurement_enabled(false), kernel_name(kernel_name),
        internal_kernel(internal_kernel) {}

  void set_verbose(bool verbose_) {
    verbose = verbose_;
    internal_kernel.get_builder()->set_verbose(verbose_);
  }

  void set_write_measurement(const std::string &scenario_name) {
    // close last scnario if there was one
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

  void set_source_inline(const std::string &source_) {
    internal_kernel.set_source_inline(source_);
  }

  void set_source_dir(const std::string &source_dir_) {
    internal_kernel.set_source_dir(source_dir_);
  }

  bool has_source() { return internal_kernel.has_source(); }

  bool has_inline_source() { return internal_kernel.has_inline_source(); }

  bool is_verbose() { return verbose; }

  virtual bool is_valid_parameter_combination() override {
    auto builder = internal_kernel.get_builder();
    void *uncasted_function =
        builder->load_other_symbol("is_valid_parameter_combination");
    if (uncasted_function == nullptr) {
      return true;
    }
    bool (*decider_pointer)() =
        reinterpret_cast<decltype(decider_pointer)>(uncasted_function);
    return decider_pointer();
  }
  void set_parameter_values(parameter_value_set &new_parameter_values) {
    parameter_values.clear();
    for (auto &p : new_parameter_values) {
      parameter_values[p.first] = p.second;
    }
  }

  template <typename parameter_interface_set>
  void set_parameter_values(parameter_interface_set &parameters) {
    parameter_values.clear();
    for (size_t i = 0; i < parameters.size(); i++) {
      auto &p = parameters[i];
      // std::cout << "p name: " << p->get_name() << " value: " <<
      // p->get_value()
      // << std::endl;
      parameter_values[p->get_name()] = p->get_value();
    }
  }

  parameter_value_set get_parameter_values() { return parameter_values; }

  virtual R operator()(Args... args) override {
    return internal_kernel(std::forward<Args>(args)...);
  }

  // very useful overload for the kernel tuners, so that they don't have to
  // track source-related arguments
  void compile(const std::string &source_dir) {
    internal_kernel.compile(source_dir);
  }

  void compile_inline(const std::string &source) {
    internal_kernel.compile_inline(source);
  }

  virtual void compile() override { internal_kernel.compile(); }

  virtual bool is_compiled() override { return internal_kernel.is_compiled(); }

  // TODO: add parameter_set argument?
  virtual void create_parameter_file() override {
    if (!has_source() || has_inline_source()) {
      throw autotune_exception("no source available");
    }
    std::shared_ptr<cppjit::builder::builder> builder = get_builder();
    const std::string &source_dir = builder->get_source_dir();
    std::ofstream parameter_file(source_dir + "parameters.hpp");
    parameter_file << "#pragma once" << std::endl;
    for (auto &p : parameter_values) {
      parameter_file << "#define " << p.first << " " << p.second << "\n";
      // parameter_file << parameters[i]->to_parameter_source_line();
    }
    parameter_file.close();
  }

  void set_builder(std::shared_ptr<cppjit::builder::builder> builder_) {
    internal_kernel.set_builder(builder_);
  }

  std::shared_ptr<cppjit::builder::builder> get_builder() {
    return internal_kernel.get_builder();
  }

  template <typename builder_class>
  std::shared_ptr<builder_class> get_builder_as() {
    return internal_kernel.template get_builder_as<builder_class>();
  }

  void clear() {
    internal_kernel.clear();
    verbose = false;
    if (scenario_measurement_file.is_open()) {
      scenario_measurement_file.close();
    }
    measurement_enabled = false;
  }
};
}

#define AUTOTUNE_DECLARE_KERNEL(kernel_signature, kernel_name)                 \
  CPPJIT_DECLARE_KERNEL(kernel_signature, kernel_name)                         \
  namespace autotune {                                                         \
  extern cppjit_kernel<                                                        \
      cppjit::detail::function_traits<kernel_signature>::return_type,          \
      cppjit::detail::function_traits<kernel_signature>::args_type>            \
      kernel_name;                                                             \
  } /* namespace autotune */

#define AUTOTUNE_DEFINE_KERNEL(kernel_signature, kernel_name)                  \
  CPPJIT_DEFINE_KERNEL(kernel_signature, kernel_name)                          \
  namespace autotune {                                                         \
  cppjit_kernel<                                                               \
      cppjit::detail::function_traits<kernel_signature>::return_type,          \
      cppjit::detail::function_traits<kernel_signature>::args_type>            \
      kernel_name(#kernel_name, cppjit::kernel_name);                          \
  } /* namespace autotune */

#define AUTOTUNE_DECLARE_DEFINE_KERNEL(signature, kernel_name)                 \
  AUTOTUNE_DECLARE_KERNEL(signature, kernel_name)                              \
  AUTOTUNE_DEFINE_KERNEL(signature, kernel_name)
