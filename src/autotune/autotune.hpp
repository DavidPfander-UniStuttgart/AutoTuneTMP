#pragma once

#include <fstream>
#include <iostream>
#include <vector>

#include "cppjit/cppjit.hpp"
#include "cppjit/function_traits.hpp"

#include "parameter.hpp"
#include "tuners/bruteforce.hpp"

// #define DEFINE_KERNEL(kernel_name)
//
//

// idea: use first invocation to trigger tuning, throw away results

namespace autotune {

// base template for the following specialization
// required to do the pack-matching in the specialization
template <typename... Args> struct kernel;

template <typename R, typename... Args>
class kernel<R, cppjit::detail::pack<Args...>> {
private:
  bool verbose;
  std::string kernel_name;
  std::vector<tunable_parameter> parameters;
  std::vector<size_t> optimal_indices;

public:
  kernel(const std::string &kernel_name)
      : verbose(false), kernel_name(kernel_name) {}

  void set_verbose(bool verbose_) { verbose = verbose_; }

  void set_source_inline(const std::string &source_);

  void set_source_dir(const std::string &source_dir_);

  bool has_source();

  bool has_inline_source();

  bool is_verbose() { return verbose; }

  void add_parameter(const tunable_parameter &parameter) {
    parameters.push_back(parameter);
    optimal_indices.push_back(0);
  }

  std::vector<tunable_parameter> &get_parameters() { return parameters; }

  template <class T>
  void add_parameter(const std::string &name, const T &values) {
    parameters.emplace_back(name, values);
  }

  const std::vector<size_t> &get_optimal_indices() { return optimal_indices; }

  void set_optimal_indices(const std::vector<size_t> &new_optimal_indices) {
    optimal_indices = new_optimal_indices;
  }

  // template <class T>
  void add_parameter(const std::string &name,
                     const std::vector<std::string> &values) {
    parameters.emplace_back(name, values);
  }

  void print_parameters() {
    std::cout << "kernel_name: " << kernel_name << std::endl;
    for (const tunable_parameter &p : parameters) {
      std::cout << "name: " << p.get_name() << " values: ";
      auto values = p.get_values();
      for (size_t i = 0; i < values.size(); i++) {
        if (i > 0) {
          std::cout << ", ";
        }
        std::cout << values[i];
      }
      std::cout << std::endl;
    }
  }

  void print_values(std::vector<std::string> &values) {
    std::cout << "current values: ";
    for (size_t i = 0; i < values.size(); i++) {
      if (i > 0) {
        std::cout << ", ";
      }
      std::cout << values[i];
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

  void create_parameter_file(const std::vector<size_t> &indices) {
    if (!has_source() || has_inline_source()) {
      throw;
    }
    std::shared_ptr<cppjit::builder::builder> builder = get_builder();
    const std::string &source_dir = builder->get_source_dir();
    std::ofstream parameter_file(source_dir + "parameters.hpp");
    parameter_file << "#pragma once" << std::endl;
    for (size_t i = 0; i < parameters.size(); i++) {
        std::cout << "name: " << parameters[i].get_name() << " index: " << indices[i]
                << std::endl;
      parameter_file << parameters[i].to_parameter_source_line(indices[i]);
    }
    parameter_file.close();
    std::string cat_cmd("cat " + source_dir + "parameters.hpp");
    std::cout << "------------------ cat --------------" << std::endl;
    std::system(cat_cmd.c_str());
  }

  void set_builder(std::shared_ptr<cppjit::builder::builder> builder_);

  std::shared_ptr<cppjit::builder::builder> get_builder();

  template <class builder_class>
  std::shared_ptr<builder_class> get_builder_as();

  void clear();

  // template <class F> F f,
  void tune(Args... args) {
    std::vector<size_t> optimal =
        bruteforce(this, std::forward<Args &>(args)...);
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
  template <typename R, typename... Args>                                      \
  R autotune::kernel<R, cppjit::detail::pack<Args...>>::operator()(            \
      Args... args) {                                                          \
    return cppjit::kernel_name(std::forward<Args>(args)...);                   \
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
  template <typename builder_class>                                            \
  std::shared_ptr<builder_class>                                               \
  autotune::kernel<R, cppjit::detail::pack<Args...>>::get_builder_as() {       \
    return cppjit::kernel_name.get_builder_as<builder_class>();                \
  }                                                                            \
  template <typename R, typename... Args>                                      \
  void autotune::kernel<R, cppjit::detail::pack<Args...>>::clear() {           \
    cppjit::kernel_name.clear();                                               \
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
  namespace autotune {                                                         \
  kernel<cppjit::detail::function_traits<kernel_signature>::return_type,       \
         cppjit::detail::function_traits<kernel_signature>::args_type>         \
      kernel_name(#kernel_name);                                               \
  } /* namespace autotune */

#define AUTOTUNE_DECLARE_DEFINE_KERNEL(signature, kernel_name)                 \
  AUTOTUNE_DECLARE_KERNEL(signature, kernel_name)                              \
  AUTOTUNE_DEFINE_KERNEL(signature, kernel_name)
