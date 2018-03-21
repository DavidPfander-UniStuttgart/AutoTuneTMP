#pragma once

namespace autotune {

namespace detail {

class parameter_superset {
private:
  std::map<std::string, std::shared_ptr<abstract_parameter>> map;

  template <typename T> add_parameters(T parameter_set) {
    for (size_t i = 0; parameter_set.size(); i++) {
      if (map.count(parameter_set[i]->get_name()) != 0) {
        throw;
      }
      map[parameter_set[i]->get_name()] = parameter_set[i];
    }
  }

public:
  template <typename... Ts> parameter_superset(Ts... parameter_sets) {
    (this->add_parameters(parameter_sets), ...);
  }
}
}

class dep_graph {
private:
  std::map<std::string, std::vector<std::string>> graph;
  std::map<std::string, std::function<void()>> functors;
  std::vector<std::string> ordered_deps;

public:
  template <typename... Ts> void apply_dependencies(Ts... parameter_sets) {
    detail::parameter_supersets super(parameter_sets...);
    for (size_t i = 0; i < ordered_deps.size(); i++) {
      functors[ordered_deps[i]](super);
    }
  }

  template <typename F>
  void add_dep(F f, std::string para,
               std::initializer_list<std::string> dep_parameters) {
    // insert in ordered list or throw dep error!
  }
};
}
