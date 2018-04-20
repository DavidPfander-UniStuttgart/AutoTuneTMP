#pragma once

#include <map>
#include <set>
#include <vector>

#include "abstract_parameter.hpp"
#include "autotune_exception.hpp"
#include "parameter_superset.hpp"
#include "tuners/countable_set.hpp"

namespace autotune {

class simple_constraints {
private:
  std::function<void(parameter_superset &)> functor;
  parameter_superset super;

public:
  simple_constraints(std::function<void(parameter_superset &)> functor)
      : functor(functor) {}

  template <typename parameter_interface>
  void add_parameters(parameter_interface &set) {
    super.add_parameters(set);
  }

  void adjust() { functor(super); }

  void clear() { super = parameter_superset(); }
};
}
