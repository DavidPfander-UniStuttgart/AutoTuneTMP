#pragma once

#include "parameter_value_set.hpp"

namespace autotune {

class parameter_set : public parameter_value_set {
public:
  parameter_set() : parameter_value_set() {}
  parameter_set(const parameter_set &other) : parameter_value_set(other) {}

  parameter_set &operator=(const parameter_set &other) {
    this->clear();
    for (auto &p : other) {
      this->insert(p);
    }
    return *this;
  }
  
  template<typename T> void add_parameter(const T &p) {
    auto element = this->insert(std::pair<const std::string, std::string>(p.get_name(), p.get_value()));
    if (!element.second)
      element.first->second = p.get_value();
  }

  template<typename T> void add_parameter_list(const std::vector<std::shared_ptr<T>> &pl) {
    for (auto &p : pl) {
      add_parameter(*p);
    }
  }
};
} // namespace autotune
