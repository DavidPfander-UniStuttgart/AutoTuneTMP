#pragma once

#include <map>
#include <sstream>
#include <iostream>

namespace autotune {

using parameter_value_set = std::map<std::string, std::string>;

template <size_t i = 0, typename F, typename... Us>
void iterate_parameter_group_tuple(F f, std::tuple<Us...> &t) {
  if
    constexpr(i < sizeof...(Us)) {
      auto &ps = std::get<i>(t);
      for (size_t j = 0; j < ps.size(); j++) {
        f(ps[j]);
      }
      iterate_parameter_group_tuple<i + 1>(f, t);
    }
  else {
    // to silence unused parameter warning in last instantiation
    (void)f;
  }
}

template <size_t i = 0, typename F, typename... Us>
void iterate_parameter_groups(F f, Us &... parameter_groups) {
  auto t = std::make_tuple(std::ref(parameter_groups)...);
  iterate_parameter_group_tuple(f, t);
}

template <typename parameter_set_type>
parameter_value_set to_parameter_values(parameter_set_type &parameters) {
  parameter_value_set value_set;
  for (size_t i = 0; i < parameters.size(); i++) {
    auto &p = parameters[i];
    value_set[p->get_name()] = p->get_value();
  }
  return value_set;
}

inline void
print_parameter_values(const parameter_value_set &parameter_values) {
  std::cout << "parameter name  | ";
  bool first = true;
  for (auto &p : parameter_values) {
    if (!first) {
      std::cout << ", ";
    } else {
      first = false;
    }
    std::cout << p.first;
    int64_t padding =
        std::max(p.first.size(), p.second.size()) - p.first.size();
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
  for (auto &p : parameter_values) {
    if (!first) {
      std::cout << ", ";
    } else {
      first = false;
    }
    std::cout << p.second;
    int64_t padding =
        std::max(p.first.size(), p.second.size()) - p.second.size();
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
}
