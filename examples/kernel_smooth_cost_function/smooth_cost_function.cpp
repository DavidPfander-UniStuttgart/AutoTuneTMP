#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

#include "parameters.hpp"

AUTOTUNE_EXPORT int smooth_cost_function(int a) {
  // use quadratic cost function
  double value = std::fabs(static_cast<double>(PAR_1) - 2.0) +
                 std::fabs(static_cast<double>(PAR_2) - 3.0);
  value *= 100;
  std::this_thread::sleep_for(
      std::chrono::milliseconds(static_cast<size_t>(value)));
  std::cout << "value: " << static_cast<size_t>(value) << std::endl;

  return a + 3;
}
