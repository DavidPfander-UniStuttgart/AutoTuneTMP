#pragma once

#include <fstream>
#include <fstream>
#include <iostream>
#include <vector>

#include "cppjit/cppjit.hpp"
#include "cppjit/function_traits.hpp"

#include "parameter.hpp"
// #include "tuners/bruteforce.hpp"
// #include "tuners/line_search.hpp"
// #include "tuners/monte_carlo.hpp"
// #include "tuners/simulated_annealing.hpp"

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
  parameter_set parameters;
  std::vector<size_t> optimal_indices;

public:
  kernel(const std::string &kernel_name)
      : verbose(false), measurement_enabled(false), kernel_name(kernel_name) {}

  void set_verbose(bool verbose_);

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
    for (size_t i = 0; i < parameters.size(); i++) {
      if (i > 0) {
        scenario_measurement_file << ", ";
      }
      scenario_measurement_file << parameters[i]->get_name();
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
    for (size_t i = 0; i < parameters.size(); i++) {
      if (i > 0) {
        scenario_measurement_file << ", ";
      }
      scenario_measurement_file << parameters[i]->get_value();
    }
    scenario_measurement_file << ", " << duration_s << std::endl;
  }

  void set_source_inline(const std::string &source_);

  void set_source_dir(const std::string &source_dir_);

  bool has_source();

  bool has_inline_source();

  bool is_verbose() { return verbose; }

  bool is_valid_parameter_combination();

  void add_parameter(const std::shared_ptr<abstract_parameter> &parameter) {
    parameters.push_back(parameter);
    // optimal_indices.push_back(0);
  }

  parameter_set &get_parameters() { return parameters; }

  // template <class T>
  // void add_parameter(const std::string &name, const T &values) {
  //   parameters.emplace_back(name, values);
  // }

  // const std::vector<size_t> &get_optimal_indices() { return optimal_indices;
  // }

  // void set_optimal_indices(const std::vector<size_t> &new_optimal_indices) {
  //   optimal_indices = new_optimal_indices;
  // }

  // template <class T>
  void add_parameter(const std::string &name,
                     const std::vector<std::string> &values) {
    auto p = std::make_shared<fixed_set_parameter>(name, values);
    if (!p) {
      std::cout << "pointer p is empty!" << std::endl;
    }
    auto q = std::dynamic_pointer_cast<abstract_parameter>(p);
    if (!q) {
      std::cout << "pointer q is empty!" << std::endl;
    }
    parameters.push_back(q);
  }

  void replace_parameters(parameter_set &new_parameters) {
    parameters = new_parameters;
  }

  void print_parameters() {
    std::cout << "kernel_name: " << kernel_name << std::endl;
    for (const std::shared_ptr<abstract_parameter> &p : parameters) {
      std::cout << "name: " << p->get_name() << " values: ";
      if (auto p_fixed_set =
              std::dynamic_pointer_cast<fixed_set_parameter>(p)) {
        auto values = p_fixed_set->get_values();
        for (size_t i = 0; i < values.size(); i++) {
          if (i > 0) {
            std::cout << ", ";
          }
          std::cout << values[i];
        }
      }
      std::cout << std::endl;
    }
  }

  void print_values() {
    // std::vector<size_t> padding(indices.size());
    // for (size_t i = 0; i < parameters.size(); i++) {
    //   padding[i] = std::max(parameters[i]->get_name().size(),
    //                         parameters[i]->get_value(indices[i]).size());
    // }
    std::cout << "parameter name  | ";
    for (size_t i = 0; i < parameters.size(); i++) {
      if (i > 0) {
        std::cout << ", ";
      }
      const std::string &name = parameters[i]->get_name();
      std::cout << name;
      //   // add padding
      //   size_t cur_padding = padding[i] - name.size();

      //   std::stringstream ss;
      //   for (size_t j = 0; j < cur_padding; j++) {
      //     ss << " ";
      //   }
      //   std::cout << ss.str();
    }
    std::cout << std::endl;
    std::cout << "parameter value | ";
    for (size_t i = 0; i < parameters.size(); i++) {
      if (i > 0) {
        std::cout << ", ";
      }
      const std::string &value = parameters[i]->get_value();
      std::cout << value;
      // // add padding
      // size_t cur_padding = padding[i] - value.size();
      // std::stringstream ss;
      // for (size_t j = 0; j < cur_padding; j++) {
      //   ss << " ";
      // }
      // std::cout << ss.str();
    }
    std::cout << std::endl;
  }

  R operator()(Args... args);

  // very useful overload for the kernel tuners, so that they don't have to
  // track source-related arguments
  void compile();

  void compile(const std::string &source_dir);

  void compile_inline(const std::string &source);

  bool is_compiled();

  // TODO: add parameter_set argument?
  void create_parameter_file() {
    if (!has_source() || has_inline_source()) {
      throw;
    }
    std::shared_ptr<cppjit::builder::builder> builder = get_builder();
    const std::string &source_dir = builder->get_source_dir();
    std::ofstream parameter_file(source_dir + "parameters.hpp");
    parameter_file << "#pragma once" << std::endl;
    for (size_t i = 0; i < parameters.size(); i++) {
      parameter_file << parameters[i]->to_parameter_source_line();
    }
    parameter_file.close();
  }

  void set_builder(std::shared_ptr<cppjit::builder::builder> builder_);

  std::shared_ptr<cppjit::builder::builder> get_builder();

  template <class builder_class>
  std::shared_ptr<builder_class> get_builder_as();

  void clear();
};
}

#define AUTOTUNE_DECLARE_KERNEL(kernel_signature, kernel_name)                 \
  CPPJIT_DECLARE_KERNEL(kernel_signature, kernel_name)                         \
  namespace autotune {                                                         \
  extern kernel<                                                               \
      cppjit::detail::function_traits<kernel_signature>::return_type,          \
      cppjit::detail::function_traits<kernel_signature>::args_type>            \
      kernel_name;                                                             \
  } /* namespace autotune */                                                   \
  template <typename R, typename... Args>                                      \
  template <typename builder_class>                                            \
  std::shared_ptr<builder_class>                                               \
  autotune::kernel<R, cppjit::detail::pack<Args...>>::get_builder_as() {       \
    return cppjit::kernel_name.get_builder_as<builder_class>();                \
  }                                                                            \
  template <typename R, typename... Args>                                      \
  R autotune::kernel<R, cppjit::detail::pack<Args...>>::operator()(            \
      Args... args) {                                                          \
    return cppjit::kernel_name(std::forward<Args>(args)...);                   \
  }                                                                            \
  template <typename R, typename... Args>                                      \
  void autotune::kernel<R, cppjit::detail::pack<Args...>>::set_verbose(        \
      bool verbose_) {                                                         \
    verbose = verbose_;                                                        \
    cppjit::kernel_name.get_builder()->set_verbose(verbose_);                  \
  }                                                                            \
  template <typename R, typename... Args>                                      \
  void autotune::kernel<R, cppjit::detail::pack<Args...>>::compile(            \
      const std::string &source_dir) {                                         \
    cppjit::kernel_name.compile(source_dir);                                   \
  }                                                                            \
  template <typename R, typename... Args>                                      \
  void autotune::kernel<R, cppjit::detail::pack<Args...>>::compile_inline(     \
      const std::string &source) {                                             \
    cppjit::kernel_name.compile_inline(source);                                \
  }                                                                            \
  template <typename R, typename... Args>                                      \
  void autotune::kernel<R, cppjit::detail::pack<Args...>>::compile() {         \
    cppjit::kernel_name.compile();                                             \
  }                                                                            \
  template <typename R, typename... Args>                                      \
  bool autotune::kernel<R, cppjit::detail::pack<Args...>>::is_compiled() {     \
    return cppjit::kernel_name.is_compiled();                                  \
  }                                                                            \
  template <typename R, typename... Args>                                      \
  void autotune::kernel<R, cppjit::detail::pack<Args...>>::set_builder(        \
      std::shared_ptr<cppjit::builder::builder> builder_) {                    \
    cppjit::kernel_name.set_builder(builder_);                                 \
  }                                                                            \
  template <typename R, typename... Args>                                      \
  std::shared_ptr<cppjit::builder::builder>                                    \
  autotune::kernel<R, cppjit::detail::pack<Args...>>::get_builder() {          \
    return cppjit::kernel_name.get_builder();                                  \
  }                                                                            \
  template <typename R, typename... Args>                                      \
  void autotune::kernel<R, cppjit::detail::pack<Args...>>::clear() {           \
    cppjit::kernel_name.clear();                                               \
    verbose = false;                                                           \
    parameters.clear();                                                        \
    optimal_indices.clear();                                                   \
    if (scenario_measurement_file.is_open()) {                                 \
      scenario_measurement_file.close();                                       \
    }                                                                          \
    measurement_enabled = false;                                               \
  }                                                                            \
  template <typename R, typename... Args>                                      \
  void autotune::kernel<R, cppjit::detail::pack<Args...>>::set_source_inline(  \
      const std::string &source_) {                                            \
    cppjit::kernel_name.set_source_inline(source_);                            \
  }                                                                            \
  template <typename R, typename... Args>                                      \
  void autotune::kernel<R, cppjit::detail::pack<Args...>>::set_source_dir(     \
      const std::string &source_dir_) {                                        \
    cppjit::kernel_name.set_source_dir(source_dir_);                           \
  }                                                                            \
  template <typename R, typename... Args>                                      \
  bool autotune::kernel<R, cppjit::detail::pack<Args...>>::has_source() {      \
    return cppjit::kernel_name.has_source();                                   \
  }                                                                            \
  template <typename R, typename... Args>                                      \
  bool                                                                         \
  autotune::kernel<R, cppjit::detail::pack<Args...>>::has_inline_source() {    \
    return cppjit::kernel_name.has_inline_source();                            \
  }                                                                            \
  template <typename R, typename... Args>                                      \
  bool autotune::kernel<                                                       \
      R, cppjit::detail::pack<Args...>>::is_valid_parameter_combination() {    \
    auto builder = autotune::kernel_name.get_builder();                        \
    void *uncasted_function =                                                  \
        builder->load_other_symbol("is_valid_parameter_combination");          \
    if (uncasted_function == nullptr) {                                        \
      return true;                                                             \
    }                                                                          \
    bool (*decider_pointer)() =                                                \
        reinterpret_cast<decltype(decider_pointer)>(uncasted_function);        \
    return decider_pointer();                                                  \
  }

// std::function<bool()> decider;
// decider = decider_pointer;

#define AUTOTUNE_DEFINE_KERNEL(kernel_signature, kernel_name)                  \
  CPPJIT_DEFINE_KERNEL(kernel_signature, kernel_name)                          \
  namespace autotune {                                                         \
  kernel<cppjit::detail::function_traits<kernel_signature>::return_type,       \
         cppjit::detail::function_traits<kernel_signature>::args_type>         \
      kernel_name(#kernel_name);                                               \
  } /* namespace autotune */

#define AUTOTUNE_DECLARE_DEFINE_KERNEL(signature, kernel_name)                 \
  AUTOTUNE_DECLARE_KERNEL(signature, kernel_name)                              \
  AUTOTUNE_DEFINE_KERNEL(signature, kernel_name)
