#pragma once

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
  std::string kernel_name;
  std::vector<tunable_parameter> parameters;
  std::vector<size_t> optimal_indices;

public:
  kernel(const std::string &kernel_name) : kernel_name(kernel_name) {}

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

  // template <class F> F f,
  void tune(Args... args) {
    std::vector<size_t> optimal = bruteforce(this, std::forward<Args>(args)...);
  }
};
}

//   R kernel<R, Args...>::operator()(Args... args) {}
//     return cppjit::kernel_name(std::forward(args)...);

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
  namespace autotune {                                                         \
  kernel<cppjit::detail::function_traits<kernel_signature>::return_type,       \
         cppjit::detail::function_traits<kernel_signature>::args_type>         \
      kernel_name(#kernel_name);                                               \
  } /* namespace autotune */

#define AUTOTUNE_DECLARE_DEFINE_KERNEL(signature, kernel_name)                 \
  AUTOTUNE_DECLARE_KERNEL(signature, kernel_name)                              \
  AUTOTUNE_DEFINE_KERNEL(signature, kernel_name)

// constructor of struct is used to run code before main
// TODO: turn this into a member function
// #define AUTOTUNE_ADD_PARAMETER(kernel_name, parameter_name, values)
//   namespace autotune {
//   namespace parameters {
//   struct add_parameter_##kernel_name_##parameter_name {
//     add_parameter_##kernel_name_##parameter_name() {
//       autotune::kernel_name.add_parameter(
//           autotune::tunable_parameter(#parameter_name, values));
//     }
//   } add_parameter_##kernel_name_##parameter_name_instance;
//   } /* namespace parameters */
//   } /* namespace autotune */
