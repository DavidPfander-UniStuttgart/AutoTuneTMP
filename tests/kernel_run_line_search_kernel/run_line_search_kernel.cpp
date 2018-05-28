#include <chrono>
#include <iostream>
#include <thread>

#include "autotune_kernel.hpp"

constexpr bool is_equal(const char *a, const char *b) {
  bool char_same = (*a == *b);
  if (!char_same) {
    return false;
  } else if (*a == '\0' && *b == '\0') {
    return true;
  } else {
    return is_equal(a + 1, b + 1);
  }
}

AUTOTUNE_EXPORT int run_line_search_kernel(int a) {
  std::cout << "PAR_1: " << PAR_1 << std::endl;
  std::cout << "PAR_2: " << PAR_2 << std::endl;
  if
    constexpr(is_equal(PAR_1, "zwei") && PAR_2 == 2) {
      std::cout << "fast kernel" << std::endl;
    }
  else if
    constexpr(is_equal(PAR_1, "zwei") || PAR_2 == 2) {
      std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
  else {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  return a + 3;
}
