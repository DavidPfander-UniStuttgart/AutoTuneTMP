#include <iostream>

#include "autotune/autotune.hpp"
#include "autotune/tuners/group_tuner.hpp"
#include "autotune/tuners/neighborhood_search.hpp"

// defines kernel, put in single compilation unit
AUTOTUNE_KERNEL(int(int), smooth_cost_function,
                "examples/kernel_smooth_cost_function")

using namespace autotune;

int main(void) {

  autotune::smooth_cost_function.set_verbose(true);

  autotune::countable_set parameters_group_1;
  autotune::countable_set parameters_group_2;
  parameters_group_1.emplace_parameter<countable_continuous_parameter>(
      "PAR_1", 3, 1, 1, 5);

  parameters_group_2.emplace_parameter<countable_continuous_parameter>(
      "PAR_2", 2, 1, 1, 5);

  iterate_parameter_groups([](auto &parameter) { parameter->set_min(); },
                           parameters_group_1, parameters_group_2);

  autotune::tuners::neighborhood_search t1(autotune::smooth_cost_function,
                                           parameters_group_1, 1);
  autotune::tuners::neighborhood_search t2(autotune::smooth_cost_function,
                                           parameters_group_2, 1);

  autotune::tuners::group_tuner g(autotune::smooth_cost_function, 2, t1, t2);
  g.set_verbose(true);
  int a = 5;
  autotune::parameter_value_set optimal_parameter_values = g.tune(a);
  autotune::smooth_cost_function.set_parameter_values(optimal_parameter_values);
}
