#include "autotune/autotune.hpp"

// defines kernel, put in single compilation unit
AUTOTUNE_KERNEL(int(int), add_one, "examples/kernel_minimal")

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>

#include <Vc/Vc>
using Vc::double_v;

int main(void) {
  autotune::add_one.set_verbose(true);

  auto kernel_copy(autotune::add_one);

  autotune::add_one.set_verbose(true);
  autotune::add_one(1);

  kernel_copy.set_verbose(true);
  kernel_copy(2);
}
