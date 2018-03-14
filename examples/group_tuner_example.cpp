#include <iostream>

#include "autotune/autotune.hpp"
#include "autotune/tuners/group_tuner.hpp"
#include "autotune/tuners/neighborhood_search.hpp"

// defines kernel, put in single compilation unit
AUTOTUNE_KERNEL(int(int), smooth_cost_function,
                "examples/kernel_smooth_cost_function")

int main(void) {

  autotune::smooth_cost_function.set_verbose(true);
  // autotune::smooth_cost_function.get_builder<cppjit::builder::gcc>()
  //     .set_verbose(true);

  autotune::countable_continuous_parameter p1("PAR_1", 3, 1, 1, 5);
  autotune::countable_continuous_parameter p2("PAR_2", 2, 1, 1, 5);

  autotune::countable_set parameters_group_1;
  autotune::countable_set parameters_group_2;
  parameters_group_1.add_parameter(p1);
  parameters_group_2.add_parameter(p2);

  iterate_parameter_groups([](auto &parameter) { parameter->set_min(); },
                           parameters_group_1, parameters_group_2);

  iterate_parameter_groups(
      [](auto &parameter) {
        std::cout << parameter->get_name()
                  << " value: " << parameter->get_value() << std::endl;
      },
      parameters_group_1, parameters_group_2);

  // make sure that all parameters are known to the kernel
  // TODO: should not be necessary -> improve!
  autotune::smooth_cost_function.set_parameter_values(parameters_group_1,
                                                      parameters_group_2);

  autotune::tuners::neighborhood_search t1(autotune::smooth_cost_function,
                                           parameters_group_1, 1);
  t1.set_verbose(true);
  autotune::tuners::neighborhood_search t2(autotune::smooth_cost_function,
                                           parameters_group_2, 1);
  t2.set_verbose(true);

  // autotune::tuners::group_tuner<autotune::countable_set, int, int>
  // g(autotune::smooth_cost_function, 1, t1, t2);
  autotune::tuners::group_tuner g(autotune::smooth_cost_function, 2, t1, t2);
  g.set_verbose(true);
  int a = 5;
  autotune::parameter_value_set optimal_parameter_values = g.tune(a);
  autotune::smooth_cost_function.set_parameter_values(optimal_parameter_values);
}
