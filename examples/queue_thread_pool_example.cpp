#include "autotune/execution_wrapper.hpp"
#include "autotune/queue_thread_pool.hpp"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>
#include <thread>

// std::mutex print_mutex;

int a = 2;

int main(void) {
  std::function<void(int)> my_function = [](int a) {
    using namespace std::chrono_literals;
    // std::unique_lock<std::mutex> lock(print_mutex);
    // std::cout << "meta.x: " << meta.x << std::endl;
    std::cout << "hello from my_function: " << a << std::endl << std::flush;
    // std::this_thread::sleep_for(2s);
  };

  std::function<void(int)> my_function_without_meta = [](int a) {
    // std::unique_lock<std::mutex> lock(print_mutex);
    std::cout << "without meta" << std::endl;
    std::cout << "hello from my_function: " << a << std::endl;
  };

  // autotune::detail::delayed_executor exe(my_function_without_meta, 3);
  // exe();

  autotune::queue_thread_pool<4> pool;
  pool.start();

  pool.enqueue_work(my_function, 1);
  pool.enqueue_work(my_function, 2);
  pool.enqueue_work(my_function, 3);
  pool.enqueue_work(my_function_without_meta, 666);
  pool.enqueue_work(my_function, 4);
  pool.enqueue_work(my_function, 5);

  pool.finish();
}
