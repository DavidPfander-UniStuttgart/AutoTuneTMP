#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>
#include <vector>

#include "autotune_kernel.hpp"

const std::vector<double> map = {
    100, 150, 30,  150, 150, //
    90,  150, 150, 40,  150, //
    80,  70,  150, 50,  150, //
    150, 150, 60,  150, 150, //
    150, 150, 150, 150, 150, //
};

AUTOTUNE_EXPORT int run_full_neighborhood_search_kernel(int a) {
  // use quadratic cost function
  // double value = std::fabs(static_cast<double>(PAR_1) - 2.0) +
  //                std::fabs(static_cast<double>(PAR_2) - 3.0);
  // value *= 100;
  double value = map[(PAR_1 - 1) * 5 + (PAR_2 - 1)];
  std::cout << "PAR_1: " << PAR_1 << " PAR_2: " << PAR_2 << " value: " << value
            << " flat_index: " << (PAR_1 * 5 + PAR_2) << std::endl;
  std::this_thread::sleep_for(
      std::chrono::milliseconds(static_cast<size_t>(value)));
  return a + 3;
}
