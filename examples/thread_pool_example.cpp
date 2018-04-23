#include "autotune/thread_pool.hpp"
#include "autotune/execution_wrapper.hpp"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>

std::mutex print_mutex;

int a = 2;
int b = 1;

int main(void) {
  std::function<void(autotune::thread_meta, int32_t)> my_function =
      [](autotune::thread_meta meta, int32_t a) {
        std::cout << "meta.x: " << meta.x << std::endl;
        std::cout << "hello from my_function: " << a << std::endl;
      };

  std::function<void(int32_t)> my_function_without_meta = [](int32_t a) {
    std::cout << "without meta" << std::endl;
    std::cout << "hello from my_function: " << a << std::endl;
  };

  // delayed_executor exe(my_function, 3);
  // exe();

  autotune::simple_thread_pool<2> pool;
  pool.start();

  pool.enqueue_work(my_function, 1);
  pool.enqueue_work(my_function, 2);
  pool.enqueue_work(my_function, 3);
  pool.enqueue_work(my_function, 4);
  pool.enqueue_work(my_function, 5);
  pool.enqueue_work(my_function_without_meta, 666);

  pool.finish();
}
