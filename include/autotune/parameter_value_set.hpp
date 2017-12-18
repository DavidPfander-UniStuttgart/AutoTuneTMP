#pragma once

#include <map>

#include "abstract_parameter.hpp"

namespace autotune {

using parameter_value_set = std::map<std::string, std::string>;

void print_parameter_values(parameter_value_set &parameter_values) {
  std::cout << "parameter name  | ";
  bool first = true;
  for (auto &p : parameter_values) {
    if (!first) {
      std::cout << ", ";
    } else {
      first = false;
    }
    std::cout << p.first;
  }
  std::cout << std::endl;
  std::cout << "parameter value | ";
  first = true;
  for (auto &p : parameter_values) {
    if (!first) {
      std::cout << ", ";
    } else {
      first = false;
    }
    std::cout << p.second;
  }
  std::cout << std::endl;
}
}
