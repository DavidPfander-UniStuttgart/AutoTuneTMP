#pragma once

#include <fstream>
#include <fstream>
#include <iostream>
#include <vector>

#include "cppjit/cppjit.hpp"
#include "cppjit/function_traits.hpp"

#include "abstract_parameter.hpp"
#include "autotune_exception.hpp"
#include "continuous_parameter.hpp"
#include "fixed_set_parameter.hpp"
#include "parameter_set.hpp"

namespace autotune {

enum class tuner { bruteforce, simulated_annealing, line_search, monte_carlo };

// base template for the following specialization
// required to do the pack-matching in the specialization
template <typename... Args> struct kernel;

template <typename R, typename... Args>
class kernel<R, cppjit::detail::pack<Args...>> {
private:
  bool verbose;
  bool measurement_enabled;
  std::ofstream scenario_measurement_file;

  std::string kernel_name;
  cppjit::kernel<R, cppjit::detail::pack<Args...>> &internal_kernel;
  // parameter_set parameters;
  parameter_value_set parameter_values;
  std::vector<size_t> optimal_indices;

public:
  kernel(const std::string &kernel_name,
         cppjit::kernel<R, cppjit::detail::pack<Args...>> &internal_kernel)
      : verbose(false), measurement_enabled(false), kernel_name(kernel_name),
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
  void write_measurement( // const std::vector<size_t> &indices,
      double duration_s) {
    if (!measurement_enabled) {
      return;
    }
    // if (indices.size() != parameters.size()) {
    //   throw;
    // }
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

  bool is_valid_parameter_combination() {
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

  // void add_parameter(const std::shared_ptr<abstract_parameter> &parameter) {
  //   parameters.push_back(parameter);
  // }

  // parameter_set &get_parameters() { return parameters; }

  // template <class T>
  // void add_parameter(const std::string &name, const T &values) {
  //   parameters.emplace_back(name, values);
  // }

  // const std::vector<size_t> &get_optimal_indices() { return
  // optimal_indices;
  // }

  // void set_optimal_indices(const std::vector<size_t> &new_optimal_indices)
  // {
  //   optimal_indices = new_optimal_indices;
  // }

  // void add_parameter(const std::string &name,
  //                    const std::vector<std::string> &values) {
  //   auto p = std::make_shared<fixed_set_parameter>(name, values);
  //   auto q = std::dynamic_pointer_cast<abstract_parameter>(p);
  //   parameters.push_back(q);
  // }

  // void add_parameter(const std::string &name, const double initial,
  //                    const double min, const double max, const double step)
  //                    {
  //   auto p = factory::make_continuous_parameter(name, initial, min, max,
  //   step);
  //   auto q = std::dynamic_pointer_cast<abstract_parameter>(p);
  //   parameters.push_back(q);
  // }

  // void add_parameter(const std::string &name, const double initial,
  //                    const double step) {
  //   auto p = factory::make_continuous_parameter(name, initial, step);
  //   auto q = std::dynamic_pointer_cast<abstract_parameter>(p);
  //   parameters.push_back(q);
  // }

  // void add_parameter(const std::string &name, const double initial) {
  //   auto p = factory::make_continuous_parameter(name, initial);
  //   auto q = std::dynamic_pointer_cast<abstract_parameter>(p);
  //   parameters.push_back(q);
  // }

  // void add_parameter(const std::string &name, const double initial,
  //                    const double min, const double max) {
  //   auto p = factory::make_continuous_parameter(name, initial, min, max);
  //   auto q = std::dynamic_pointer_cast<abstract_parameter>(p);
  //   parameters.push_back(q);
  // }

  // void set_parameters(parameter_set &new_parameters) {
  //   parameters = new_parameters;
  // }

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

  // void print_parameter_values() {
  //   std::cout << "kernel_name: " << kernel_name << std::endl;
  //   // for (const std::shared_ptr<abstract_parameter> &p : parameters) {
  //   for (auto &p: parameter_values) {
  //     std::cout << "name: " << p.first << " values: ";
  //     if (auto p_fixed_set =
  //             std::dynamic_pointer_cast<fixed_set_parameter>(p)) {
  //       auto values = p_fixed_set->get_values();
  //       for (size_t i = 0; i < values.size(); i++) {
  //         if (i > 0) {
  //           std::cout << ", ";
  //         }
  //         std::cout << values[i];
  //       }
  //     }
  //     std::cout << std::endl;
  //   }
  // }

  R operator()(Args... args) {
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

  void compile() { internal_kernel.compile(); }

  bool is_compiled() { return internal_kernel.is_compiled(); }

  // TODO: add parameter_set argument?
  void create_parameter_file() {
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
    optimal_indices.clear();
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
  extern kernel<                                                               \
      cppjit::detail::function_traits<kernel_signature>::return_type,          \
      cppjit::detail::function_traits<kernel_signature>::args_type>            \
      kernel_name;                                                             \
  } /* namespace autotune */

#define AUTOTUNE_DEFINE_KERNEL(kernel_signature, kernel_name)                  \
  CPPJIT_DEFINE_KERNEL(kernel_signature, kernel_name)                          \
  namespace autotune {                                                         \
  kernel<cppjit::detail::function_traits<kernel_signature>::return_type,       \
         cppjit::detail::function_traits<kernel_signature>::args_type>         \
      kernel_name(#kernel_name, cppjit::kernel_name);                          \
  } /* namespace autotune */

#define AUTOTUNE_DECLARE_DEFINE_KERNEL(signature, kernel_name)                 \
  AUTOTUNE_DECLARE_KERNEL(signature, kernel_name)                              \
  AUTOTUNE_DEFINE_KERNEL(signature, kernel_name)
