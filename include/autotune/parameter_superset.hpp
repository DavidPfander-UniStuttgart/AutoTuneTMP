#pragma once

#include <map>

#include "autotune_exception.hpp"

namespace autotune {

class parameter_superset : std::map<std::string, void *> {
public:
  template <typename... Ts> parameter_superset(Ts &... parameter_sets) {
    (this->add_parameters(parameter_sets), ...);
  }

  template <typename T> parameter_superset(std::vector<T> &parameter_sets) {
    for (size_t i = 0; i < parameter_sets.size(); i++) {
      this->add_parameters(parameter_sets);
    }
  }

  using std::map<std::string, void *>::size;

  template <typename T> T &get_as(const std::string &name) {
    T *casted = static_cast<T *>(this->operator[](name));
    return *casted;
  }

  template <typename T>
  void add_parameters(T &parameter_set, bool overwrite = false) {
    for (size_t i = 0; i < parameter_set.size(); i++) {
      if (this->count(parameter_set[i]->get_name()) != 0) {
        if (!overwrite) {
          throw autotune_exception(std::string("parameter in multiple sets: ") +
                                   parameter_set[i]->get_name());
        } else {
          std::cout << "parameter_superset updates parameter: "
                    << parameter_set[i]->get_name() << std::endl;
        }
      }
      this->operator[](parameter_set[i]->get_name()) =
          parameter_set[i]->get_void_ptr();
    }
  }
};
}
