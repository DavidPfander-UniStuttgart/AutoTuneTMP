#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

#include "autotune_kernel.hpp"

AUTOTUNE_EXPORT int run_neighborhood_search_kernel(int a) {
  // use quadratic cost function
  double value = std::fabs(static_cast<double>(PAR_1) - 2.0) +
                 std::fabs(static_cast<double>(PAR_2) - 3.0);
  value *= 100;
  std::this_thread::sleep_for(
      std::chrono::milliseconds(static_cast<size_t>(value)));
  return a + 3;
}
