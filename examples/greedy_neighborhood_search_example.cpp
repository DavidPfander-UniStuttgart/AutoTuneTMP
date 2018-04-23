#include <algorithm>
#include <iostream>
#include <vector>

#include "autotune/autotune.hpp"
#include "autotune/parameter.hpp"
#include "autotune/tuners/greedy_neighborhood_search.hpp"

// defines kernel, put in single compilation unit
AUTOTUNE_KERNEL(int(int), smooth_cost_function,
                "examples/kernel_smooth_cost_function")

int main(void) {
  std::cout << "testing countable set interface" << std::endl;
  autotune::smooth_cost_function.set_verbose(false);

  autotune::countable_set parameters;
  autotune::countable_continuous_parameter p1("PAR_1", 1.0, 1.0, 1.0, 5.0);
  parameters.add_parameter(p1);

  autotune::countable_continuous_parameter p2("PAR_2", 1.0, 1.0, 1.0, 5.0);
  parameters.add_parameter(p2);

  std::function<bool(int)> test_result = [](int) -> bool {
    /* tests values generated by kernel */
    return true;
  };

  int a = 5;

  autotune::tuners::greedy_neighborhood_search tuner(
      autotune::smooth_cost_function, parameters, 20, 2);
  tuner.setup_test(test_result);
  tuner.set_verbose(true);
  autotune::countable_set optimal_parameters = tuner.tune(a);
  autotune::smooth_cost_function.set_parameter_values(optimal_parameters);
  optimal_parameters.print_values();

  return 0;
}