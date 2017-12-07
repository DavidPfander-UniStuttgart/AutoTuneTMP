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
}
