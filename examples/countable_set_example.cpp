#include <algorithm>
#include <iostream>
#include <vector>

#include "autotune/autotune.hpp"
#include "autotune/parameter.hpp"
#include "autotune/tuners/bruteforce.hpp"
#include "autotune/tuners/countable_set.hpp"

// defines kernel, put in single compilation unit
AUTOTUNE_KERNEL(int(int), add_one, "examples/kernel_minimal")

int main(void) {
  std::cout << "testing countable set interface" << std::endl;

  autotune::countable_set parameters;
  autotune::fixed_set_parameter<std::string> p1("PAR_1",
                                                {"eins", "zwei", "drei"});
  parameters.add_parameter(p1);

  autotune::countable_continuous_parameter p2("PAR_2", 1.0, 1.0, 1.0, 5.0);
  parameters.add_parameter(p2);

  std::function<bool(int)> test_result = [](int) -> bool {
    /* tests values generated by kernel */
    return true;
  };

  int a = 5;

  autotune::tuners::bruteforce tuner(autotune::add_one, parameters);
  tuner.setup_test(test_result);
  tuner.set_verbose(true);
  autotune::countable_set optimal_parameters = tuner.tune(a);
  autotune::add_one.set_parameter_values(optimal_parameters);

  return 0;
}
