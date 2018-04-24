#include "autotune/execution_wrapper.hpp"
#include "autotune/thread_pool.hpp"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>

std::mutex print_mutex;

int a = 2;

int main(void) {
  std::function<void(autotune::thread_meta, int32_t)> my_function =
      [](autotune::thread_meta meta, int32_t a) {
        std::unique_lock<std::mutex> lock(print_mutex);
        std::cout << "meta.x: " << meta.x << std::endl;
        std::cout << "hello from my_function: " << a << std::endl;
      };

  std::function<void(int32_t)> my_function_without_meta = [](int32_t a) {
    std::unique_lock<std::mutex> lock(print_mutex);
    std::cout << "without meta" << std::endl;
    std::cout << "hello from my_function: " << a << std::endl;
  };

  autotune::detail::delayed_executor exe(my_function_without_meta, 3);
  exe();

  autotune::detail::thread_safe_queue<size_t> safe_q;
  safe_q.push_back(8);

  // autotune::simple_thread_pool<2> pool;
  // pool.start();

  // pool.enqueue_work(my_function, 1);
  // pool.enqueue_work(my_function, 2);
  // pool.enqueue_work(my_function, 3);
  // pool.enqueue_work(my_function_without_meta, 666);
  // pool.enqueue_work(my_function, 4);
  // for (size_t i = 0; i < 1000000; i++) {
  //   pool.enqueue_work(my_function, 5);
  // }

  // print_mutex.lock();
  // std::cout << "finished queuing work" << std::endl;
  // print_mutex.unlock();

  // pool.finish();
}
