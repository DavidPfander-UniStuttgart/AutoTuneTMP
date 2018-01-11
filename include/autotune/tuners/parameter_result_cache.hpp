#pragma once

#include <map>

namespace autotune {

template <typename parameter_interface> class node {
public:
  std::map<std::string, node<parameter_interface>> children;

  void insert(parameter_interface &parameters, size_t cur_index) {
    if (cur_index == parameters.size()) {
      return;
    }

    auto child_iterator = children.find(parameters[cur_index]->get_value());
    if (child_iterator == children.end()) {
      auto pair = children.emplace(parameters[cur_index]->get_value(),
                                   node<parameter_interface>());
      if (std::get<1>(pair)) {
        child_iterator = std::get<0>(pair);
      }
    }
    child_iterator->second.insert(parameters, cur_index + 1);
  }

  bool contains(parameter_interface &candidate, size_t cur_index) {
    if (candidate.size() == cur_index) {
      return true;
    }
    auto child_iterator = children.find(candidate[cur_index]->get_value());
    if (child_iterator == children.end()) {
      return false;
    }
    return child_iterator->second.contains(candidate, cur_index + 1);
  }
};

template <typename parameter_interface>
class parameter_result_cache : public node<parameter_interface> {
private:
  using node<parameter_interface>::insert;
  using node<parameter_interface>::contains;

public:
  void insert(parameter_interface &parameters) { this->insert(parameters, 0); }

  bool contains(parameter_interface &candidate) {
    return this->contains(candidate, 0);
  }
  void clear() { this->children.clear(); };
};
}
