#include <chrono>
#include <iostream>
#include <thread>

#include "parameters.hpp"

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

extern "C" int run_bruteforce_kernel(int a) {
  if
    constexpr(is_equal(PAR_1, "eins") && PAR_2 == 2) {
      std::cout << "fast kernel" << std::endl;
    }
  else {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  return a + 3;
}
