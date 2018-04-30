// #include "autotune/grid_executor.hpp"
// #include "autotune/execution_wrapper.hpp"
// #include "autotune/queue_thread_pool.hpp"
// #include "autotune/thread_pool.hpp"
// #include "autotune/thread_safe_queue.hpp"

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
  // autotune::add_one.get_builder<cppjit::builder::gcc>().set_verbose(true);
  //
  // auto cloned = autotune::add_one.clone();
}
