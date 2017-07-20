#pragma once

#include <chrono>

namespace autotune {

template <class F, typename... Args>
void evalute(std::vector<size_t> indices, F f, Args... args) {
  auto start = std::chrono::high_resolution_clock::now();

  // call kernel, discard possibly returned values
  (*f)(args);

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> duration = end - start;
  std::cout << "kernel call duration (ms): " << duration.count() << std::endl;
}
}
