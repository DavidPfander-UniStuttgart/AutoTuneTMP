#pragma once

#include <map>

namespace autotune {

class parameter_superset : std::map<std::string, void *> {
private:
  // std::map<std::string, std::shared_ptr<abstract_parameter>> map;

  template <typename T> void add_parameters(T &parameter_set) {
    for (size_t i = 0; i < parameter_set.size(); i++) {
      if (this->count(parameter_set[i]->get_name()) != 0) {
        throw autotune_exception(std::string("parameter in multiple sets: ") +
                                 parameter_set[i]->get_name());
      }
      this->operator[](parameter_set[i]->get_name()) =
          parameter_set[i]->get_void_ptr();
    }
  }

public:
  template <typename... Ts> parameter_superset(Ts &... parameter_sets) {
    (this->add_parameters(parameter_sets), ...);
  }

  using std::map<std::string, void *>::size;

  template <typename T> T &get_as(const std::string &name) {
    T *casted = static_cast<T *>(this->operator[](name));
    return *casted;
  }
};
}
