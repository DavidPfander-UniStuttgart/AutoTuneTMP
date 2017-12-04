#pragma once

#include "abstract_parameter.hpp"

namespace autotune {

// using parameter_set = std::vector<std::shared_ptr<abstract_parameter>>;
class parameter_set : public std::vector<std::shared_ptr<abstract_parameter>> {
public:
  // void operator=(parameter_set const &other) {
  //   for (size_t i = 0; i < other.size(); i++) {
  //     this->push_back(other[i]);
  //   }
  // }

  parameter_set clone() {
    parameter_set new_instance;
    for (size_t i = 0; i < this->size(); i++) {
      new_instance.push_back(this->operator[](i)->clone());
    }
    return new_instance;
  }
};
}
