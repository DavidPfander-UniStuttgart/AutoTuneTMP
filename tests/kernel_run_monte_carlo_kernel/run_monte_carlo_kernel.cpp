#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

#include "parameters.hpp"

AUTOTUNE_EXPORT int run_monte_carlo_kernel(int a) {
  if
    constexpr(PAR_1 == 1.000000 && PAR_2 == 1.000000) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  return a + 3;
}
