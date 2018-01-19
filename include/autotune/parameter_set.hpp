#pragma once

#include "abstract_parameter.hpp"

namespace autotune {

class parameter_set : public std::vector<std::shared_ptr<abstract_parameter>> {
public:
  parameter_set() : std::vector<std::shared_ptr<abstract_parameter>>() {}

  parameter_set(const parameter_set &other)
      : std::vector<std::shared_ptr<abstract_parameter>>() {
    for (size_t i = 0; i < other.size(); i++) {
      this->push_back(other[i]->clone_wrapper());
    }
  }

  parameter_set &operator=(const parameter_set &other) {
    this->clear();
    for (size_t i = 0; i < other.size(); i++) {
      this->push_back(other[i]->clone_wrapper());
    }
    return *this;
  }

  std::shared_ptr<abstract_parameter> get_by_name(const std::string &name) {
    for (auto p : *this) {
      if (p->get_name().compare(name) == 0) {
        return p;
      }
    }
    return nullptr;
  }

  template<typename T> void add_parameter(T &p) {
    std::shared_ptr<abstract_parameter_wrapper<T>> cloned =
        std::make_shared<abstract_parameter_wrapper<T>>(p);
    this->push_back(cloned);
  }

  void print_values() {
    std::cout << "parameter name  | ";
    bool first = true;
    for (auto &p : *this) {
      if (!first) {
        std::cout << ", ";
      } else {
        first = false;
      }
      std::cout << p->get_name();
      int64_t padding = std::max(p->get_name().size(), p->get_value().size()) -
                        p->get_name().size();
      if (padding > 0) {
        std::stringstream ss;
        for (int64_t i = 0; i < padding; i++) {
          ss << " ";
        }
        std::cout << ss.str();
      }
    }
    std::cout << std::endl;
    std::cout << "parameter value | ";
    first = true;
    for (auto &p : *this) {
      if (!first) {
        std::cout << ", ";
      } else {
        first = false;
      }
      std::cout << p->get_value();
      int64_t padding = std::max(p->get_name().size(), p->get_value().size()) -
                        p->get_value().size();
      if (padding > 0) {
        std::stringstream ss;
        for (int64_t i = 0; i < padding; i++) {
          ss << " ";
        }
        std::cout << ss.str();
      }
    }
    std::cout << std::endl;
  }
};
} // namespace autotune
