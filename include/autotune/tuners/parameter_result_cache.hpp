#pragma once

#include <map>

namespace autotune {

// template <typename parameter_interface>
class node {
public:
  std::map<std::string, node> children;

  void insert(parameter_value_set::const_iterator &parameters_it,
              parameter_value_set::const_iterator &parameters_it_end) {
    if (parameters_it == parameters_it_end) {
      return;
    }

    // std::cout << parameters_it->second << " ";

    // auto cur_value = std::next(parameters.begin(), cur_index)->second;
    auto child_iterator = children.find(parameters_it->second);
    if (child_iterator == children.end()) {
      auto pair = children.emplace(parameters_it->second, node());
      if (std::get<1>(pair)) {
        child_iterator = std::get<0>(pair);
      }
    }
    child_iterator->second.insert(++parameters_it, parameters_it_end);
  }

  bool contains(parameter_value_set::const_iterator &candidate_it,
                parameter_value_set::const_iterator &candidate_it_end) {
    if (candidate_it == candidate_it_end) {
      return true;
    }
    // auto cur_value = std::next(candidate.begin(), cur_index)->second;
    auto child_it = children.find(candidate_it->second);
    if (child_it == children.end()) {
      return false;
    }
    // std::cout << candidate_it->second << " ";
    return child_it->second.contains(++candidate_it, candidate_it_end);
  }
};

// template <typename parameter_interface>
class parameter_result_cache : public node {
private:
  using node::insert;
  using node::contains;

public:
  void insert(const parameter_value_set &parameters) {
    auto parameters_it = parameters.begin();
    auto parameters_it_end = parameters.end();
    // std::cout << "insert: ";
    this->insert(parameters_it, parameters_it_end);
    // std::cout << std::endl;
  }

  template <typename parameter_interface>
  void insert(const parameter_interface &parameters) {
    insert(to_parameter_values(parameters));
  }

  bool contains(const parameter_value_set &candidate) {
    auto candidate_it = candidate.begin();
    auto candidate_it_end = candidate.end();
    // std::cout << "contains: ";
    return this->contains(candidate_it, candidate_it_end);
    // std::cout << std::endl;
  }

  template <typename parameter_interface>
  bool contains(const parameter_interface &candidate) {
    return this->contains(to_parameter_values(candidate));
  }

  void clear() { this->children.clear(); };
};
}
