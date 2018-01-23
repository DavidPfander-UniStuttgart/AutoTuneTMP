#include <algorithm>
#include <iostream>
#include <vector>

#include "autotune/autotune.hpp"
#include "autotune/continuous_parameter.hpp"
#include "autotune/tuners/line_search.hpp"

// defines kernel, put in single compilation unit
AUTOTUNE_KERNEL(int(int), add_one, "examples/kernel_minimal")

int main(void) {
  autotune::add_one.set_verbose(true);

  // register parameters
  autotune::countable_set parameters;
  autotune::fixed_set_parameter<std::string> p1("ADD_ONE", {"0", "1"}, false);
  parameters.add_parameter(p1);

  int a = 5;

  size_t line_search_iterations = 1;
  autotune::tuners::line_search tuner(autotune::add_one, parameters,
                                      line_search_iterations, 1);
  autotune::countable_set optimal_parameters = tuner.tune(a);
  autotune::add_one.set_parameter_values(optimal_parameters);

  autotune::add_one(a);
  return 0;
}
