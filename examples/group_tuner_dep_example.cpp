#include <iostream>

#include "autotune/autotune.hpp"
#include "autotune/constraint_graph.hpp"
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
  parameters_group_2.emplace_parameter<countable_continuous_parameter>(
      "PAR_3", 2, 1, 1, 5);

  iterate_parameter_groups([](auto &parameter) { parameter->set_min(); },
                           parameters_group_1, parameters_group_2);

  autotune::constraint_graph constraints;

  constraints.add_constraint(
      "PAR_1", {"PAR_2"},
      [](auto &pars) {
        std::cout << pars.size() << std::endl;
        auto &p =
            pars.template get_as<autotune::countable_continuous_parameter>(
                "PAR_1");
        std::cout << p.get_name() << std::endl;
      });
  constraints.add_constraint(
      "PAR_2", {"PAR_3"},
      [](auto &pars) {
        std::cout << pars.size() << std::endl;
        auto &p =
            pars.template get_as<autotune::countable_continuous_parameter>(
                "PAR_2");
        std::cout << p.get_name() << std::endl;
      });
  constraints.add_constraint(
      "PAR_3", {},
      [](auto &pars) {
        std::cout << pars.size() << std::endl;
        auto &p =
            pars.template get_as<autotune::countable_continuous_parameter>(
                "PAR_3");
        std::cout << p.get_name() << std::endl;
      });

  // constraints.apply_dependencies(parameters_group_1, parameters_group_2);

  autotune::tuners::neighborhood_search t1(autotune::smooth_cost_function,
                                           parameters_group_1, 1);
  autotune::tuners::neighborhood_search t2(autotune::smooth_cost_function,
                                           parameters_group_2, 1);

  autotune::tuners::group_tuner g(autotune::smooth_cost_function, 2, t1, t2);
  // g.set_constraint_graph(constraints);
  g.set_verbose(true);
  int a = 5;
  autotune::parameter_value_set optimal_parameter_values = g.tune(a);
  autotune::smooth_cost_function.set_parameter_values(optimal_parameter_values);
}
