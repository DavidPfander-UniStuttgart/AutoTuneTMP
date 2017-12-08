#pragma once

#include <map>

#include "abstract_parameter.hpp"

namespace autotune {

using parameter_value_set = std::map<std::string, std::string>;

// using parameter_set = std::vector<std::shared_ptr<abstract_parameter>>;

// class parameter_set : public std::vector<std::shared_ptr<abstract_parameter>>
// {
// public:

//   template <typename T> std::shared_ptr<T> get_as(size_t i) {
//     return std::dynamic_pointer_cast<T>(this->operator[](i));
//   }

//   parameter_set clone() {
//     parameter_set new_instance;
//     for (size_t i = 0; i < this->size(); i++) {
//       new_instance.push_back(this->operator[](i)->clone());
//     }
//     return new_instance;
//   }
// };

void print_parameter_values(parameter_value_set &parameter_values) {
  // std::vector<size_t> padding(parameter_values.size());
  // for (auto &p : parameter_values) {
  //   padding.push_back(p.first.size());
  // }
  std::cout << "parameter name  | ";
  // for (size_t i = 0; i < parameters.size(); i++) {
  bool first = true;
  // size_t index = 0;
  for (auto &p : parameter_values) {
    if (!first) {
      std::cout << ", ";
    } else {
      first = false;
    }
    std::cout << p.first;
    // // add padding
    // size_t cur_padding = padding[index] - p.first.size();
    // std::cout << std::endl << "padding:" << padding[index] << std::endl;
    // std::cout << "p.first.size():" << p.first.size() << std::endl;
    // std::cout << "cur_padding:" << cur_padding << std::endl;

    // std::stringstream ss;
    // for (size_t j = 0; j < cur_padding; j++) {
    //   ss << " ";
    // }
    // std::cout << ss.str();
    // index += 1;
  }
  std::cout << std::endl;
  std::cout << "parameter value | ";
  // for (size_t i = 0; i < parameters.size(); i++) {
  first = true;
  // index = 0;
  for (auto &p : parameter_values) {
    if (!first) {
      std::cout << ", ";
    } else {
      first = false;
    }
    std::cout << p.second;
    // // add padding
    // size_t cur_padding = padding[index] - p.second.size();
    // std::stringstream ss;
    // for (size_t j = 0; j < cur_padding; j++) {
    //   ss << " ";
    // }
    // std::cout << ss.str();
    // index += 1;
  }
  std::cout << std::endl;
}
}
