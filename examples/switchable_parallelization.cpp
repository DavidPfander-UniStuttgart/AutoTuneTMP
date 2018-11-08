// #include "autotune/execution_wrapper.hpp"
// #include "autotune/thread_pool.hpp"
#include "opttmp/loop/parallelizable_loop.hpp"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>

std::mutex print_mutex;

int a = 2;

int main(void) {

  autotune::simple_thread_pool<4> pool;
  pool.start();

  std::tuple<size_t, size_t, size_t> loop_bounds{0, 10, 1};

  std::function<void(size_t)> body = [&pool, &loop_bounds](size_t i) {
    std::function<void(size_t)> inner_body = [&pool, i](size_t j) {
      std::cout << "hello i * j: " << i * j << std::endl;
    };
    opttmp::loop::parallizable_loop<true>(pool, loop_bounds, inner_body);
  };

  opttmp::loop::parallizable_loop<true>(pool, loop_bounds, body);

  pool.finish();
}
