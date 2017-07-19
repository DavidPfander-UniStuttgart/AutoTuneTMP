#pragma once

#include <iostream>
#include <vector>

#include "parameter.hpp"

// #define DEFINE_KERNEL(kernel_name)
//
//

// idea: use first invocation to trigger tuning, throw away results

namespace autotune {

struct kernel {
private:
  std::string kernel_name;
  std::vector<tunable_parameter> parameters;

public:
  kernel(const std::string &kernel_name) : kernel_name(kernel_name) {}

  void add_parameter(const tunable_parameter &parameter) {
    parameters.push_back(parameter);
  }

  std::vector<tunable_parameter> &get_parameters() { return parameters; }

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
};
}

#define AUTOTUNE_DEFINE_KERNEL(kernel_name)                                    \
  namespace autotune {                                                         \
  namespace kernels {                                                          \
  kernel kernel_name(#kernel_name);                                            \
  } /* namespace kernels */                                                    \
  } /* namespace autotune */

// constructor of struct is used to run code before main
#define AUTOTUNE_ADD_PARAMETER(kernel_name, parameter_name, values)            \
  namespace autotune {                                                         \
  namespace parameters {                                                       \
  struct add_parameter_##kernel_name_##parameter_name {                        \
    add_parameter_##kernel_name_##parameter_name() {                           \
      autotune::kernels::kernel_name.add_parameter(                            \
          autotune::tunable_parameter(#parameter_name, values));               \
    }                                                                          \
  } add_parameter_##kernel_name_##parameter_name_instance;                     \
  } /* namespace parameters */                                                 \
  } /* namespace autotune */
