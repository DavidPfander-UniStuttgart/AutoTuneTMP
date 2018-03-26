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

public:
  simple_constraints(std::function<void(parameter_superset &)> functor)
      : functor(functor) {}

  template <typename... Ts> void apply_dependencies(Ts... parameter_sets) {
    parameter_superset super(parameter_sets...);
    functor(super);
  }
};
}
