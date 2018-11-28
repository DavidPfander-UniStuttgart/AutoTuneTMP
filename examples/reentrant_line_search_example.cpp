#include <algorithm>
#include <iostream>
#include <vector>

#include "autotune/autotune.hpp"
#include "autotune/parameter.hpp"
#include "autotune/tuners/countable_set.hpp"
#include "autotune/tuners/reentrant_line_search.hpp"

AUTOTUNE_KERNEL(int(int), input_variant_smooth_cost_function,
                "examples/kernel_input_variant_smooth_cost_function")
int main(void) {

  autotune::countable_set parameters;
  parameters.emplace_parameter<autotune::countable_continuous_parameter>(
      "PAR_1", 1.0, 1.0, 1.0, 5.0);

  parameters.emplace_parameter<autotune::countable_continuous_parameter>(
      "PAR_2", 1.0, 1.0, 1.0, 5.0);

  int a = 1;

  autotune::tuners::reentrant_line_search tuner(
      autotune::input_variant_smooth_cost_function, parameters, 10);
  tuner.set_verbose(true);
  tuner.set_write_measurement("reentrant_line_search_demo");
  tuner.set_weigh_input_functor(
      [](int a) -> double { return 1.0 / static_cast<double>(a); });
  // run the kernel, calculate the results and at the same time use the runtime
  // information for tuning
  size_t calls = 10;
  for (size_t i = 0; i < calls; i += 1) {
    std::cout << "a: " << a << " call: " << i << std::endl;
    tuner.reentrant_tune(a);
    a += 1;
  }
  autotune::countable_set optimal_parameters = tuner.get_optimal_parameters();
  std::cout << "parameters after " << calls << " calls" << std::endl;
  optimal_parameters.print_values();
  return 0;
}
