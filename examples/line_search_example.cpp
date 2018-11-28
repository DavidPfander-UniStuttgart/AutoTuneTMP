#include <algorithm>
#include <iostream>
#include <vector>

#include "autotune/autotune.hpp"
#include "autotune/parameter.hpp"
#include "autotune/tuners/line_search.hpp"

AUTOTUNE_KERNEL(int(int), smooth_cost_function,
                "examples/kernel_smooth_cost_function")

int main(void) {
  autotune::countable_set parameters;
  parameters.emplace_parameter<autotune::countable_continuous_parameter>(
      "PAR_1", 1.0, 1.0, 1.0, 5.0);

  parameters.emplace_parameter<autotune::countable_continuous_parameter>(
      "PAR_2", 1.0, 1.0, 1.0, 5.0);

  int a = 5;

  autotune::tuners::line_search tuner(autotune::smooth_cost_function,
                                      parameters, 10);
  tuner.set_verbose(true);
  tuner.set_write_measurement("line_search_demo");
  autotune::countable_set optimal_parameters = tuner.tune(a);
  std::cout << "optimal_parameters:" << std::endl;
  optimal_parameters.print_values();

  return 0;
}
