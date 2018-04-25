#include "autotune/execution_wrapper.hpp"
#include "autotune/queue_thread_pool.hpp"
#include "autotune/thread_pool.hpp"
#include "autotune/thread_safe_queue.hpp"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>

std::mutex print_mutex;

namespace autotune {

class grid_spec {
public:
  size_t grid_z;
  size_t grid_y;
  size_t grid_x;

  size_t block_z;
  size_t block_y;
  size_t block_x;
};

template <size_t num_threads> class grid_executor {
private:
  // template <typename... Args>
  // void thread_worker(std::function<void(Args...)> f, grid_spec spec,
  //                    thread_meta base) {
  //   // [&spec, f, meta_base, grid_z, grid_y, grid_x]() {
  //   for (size_t block_z = 0; block_z < spec.block_z; block_z++) {
  //     for (size_t block_y = 0; block_y < spec.block_y; block_y++) {
  //       for (size_t block_x = 0; block_x < spec.block_x;
  //            block_x += types_width) {
  //         thread_meta meta = base;
  //         meta.z = base.z + block_z;
  //         meta.y = base.y + block_y;
  //         meta.x = base.x + block_x;
  //         f(meta);
  //       }
  //     }
  //   }
  // }

public:
  using float_type = float;
  using double_type = double;
  using int32_type = int32_t;
  using uint32_type = uint32_t;
  using int64_type = int64_t;
  using uint64_type = uint64_t;
  static constexpr size_t types_width = 1;

  template <typename... Args>
  void operator()(std::function<void(Args...)> f, grid_spec spec) {
    autotune::queue_thread_pool<num_threads> pool;

    std::function<void(grid_spec, thread_meta)> thread_wrapper =
        [f](grid_spec spec, thread_meta meta_base) {
          for (size_t block_z = 0; block_z < spec.block_z; block_z++) {
            for (size_t block_y = 0; block_y < spec.block_y; block_y++) {
              for (size_t block_x = 0; block_x < spec.block_x;
                   block_x += types_width) {
                thread_meta meta = meta_base;
                meta.z += block_z;
                meta.y += block_y;
                meta.x += block_x;
                f(meta);
              }
            }
          }
        };

    pool.start();
    for (size_t grid_z = 0; grid_z < spec.grid_z; grid_z++) {
      for (size_t grid_y = 0; grid_y < spec.grid_y; grid_y++) {
        for (size_t grid_x = 0; grid_x < spec.grid_x; grid_x++) {
          thread_meta meta_base;
          meta_base.z = grid_z * spec.block_z;
          meta_base.y = grid_y * spec.block_y;
          meta_base.x = grid_x * spec.block_x;

          pool.enqueue_work(thread_wrapper, spec, meta_base);

          // pool.enqueue_work(&grid_executor<num_threads>::thread_worker, this,
          // f,
          //                   spec, meta_base);
        }
      }
    }
    pool.finish();
  }
};
}

int a = 2;

void print_matrix(std::vector<double> m, size_t N) {
  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < N; j++) {
      if (j > 0)
        std::cout << ", ";
      std::cout << m[i * N + j];
    }
    std::cout << std::endl;
  }
}

int main(void) {
  std::function<void(autotune::thread_meta, int32_t)> my_function =
      [](autotune::thread_meta meta, int32_t a) {
        size_t result = 0;
        for (size_t i = 0; i < 1000000; i++) {
          result += i;
        }
        // std::unique_lock<std::mutex> lock(print_mutex);
        std::cout << "meta.x: " << meta.x << ", hello from my_function: " << a
                  << std::endl;
        std::cout << "result: " << result << std::endl;
      };

  std::function<void(int32_t)> my_function_without_meta = [](int32_t a) {
    // std::unique_lock<std::mutex> lock(print_mutex);
    std::cout << "without meta, hello from my_function: " << a << std::endl;
  };

  // autotune::detail::delayed_executor exe(my_function_without_meta, 3);
  // exe();

  // autotune::detail::thread_safe_queue<size_t> safe_q;
  // size_t input = 8;
  // std::cout << "is_empty? " << safe_q.reserve() << std::endl;
  // safe_q.push_back(input);
  // safe_q.push_back(input);
  // safe_q.push_back(input);
  // safe_q.push_back(input);
  // std::cout << "is_empty? " << safe_q.reserve() << std::endl;
  // size_t first_element = safe_q.next();
  // std::cout << "first_element: " << first_element << std::endl;
  // std::cout << "is_empty? " << safe_q.reserve() << std::endl;
  // first_element = safe_q.next();
  // std::cout << "first_element: " << first_element << std::endl;
  // std::cout << "is_empty? " << safe_q.reserve() << std::endl;
  // first_element = safe_q.next();
  // std::cout << "first_element: " << first_element << std::endl;
  // std::cout << "is_empty? " << safe_q.reserve() << std::endl;
  // first_element = safe_q.next();
  // std::cout << "first_element: " << first_element << std::endl;
  // std::cout << "is_empty? " << safe_q.reserve() << std::endl;

  // autotune::detail::thread_safe_queue<size_t> safe_q;
  // size_t input = 8;
  // safe_q.push_back(input);
  // safe_q.reserve();
  // size_t first_element = safe_q.next();
  // std::cout << "first_element: " << first_element << std::endl;

  // std::thread t1([&safe_q]() {
  //   for (size_t i = 0; i < 10000; i++) {
  //     // print_mutex.lock();
  //     // std::cout << "enqueue!" << std::endl;
  //     // print_mutex.unlock();
  //     // std::this_thread::sleep_for(std::chrono::nanoseconds(2));
  //     safe_q.push_back(i);
  //   }
  // });

  // std::thread t2([&safe_q]() {
  //   for (size_t i = 0; i < 10000; i++) {
  //     while (!safe_q.reserve()) {
  //       // print_mutex.lock();
  //       // std::cout << "is still empty" << std::endl;
  //       // print_mutex.unlock();
  //       // std::this_thread::sleep_for(std::chrono::nanoseconds(2));
  //     }
  //     size_t r = safe_q.next();
  //     print_mutex.lock();
  //     std::cout << "r: " << r << std::endl;
  //     print_mutex.unlock();
  //   }
  // });

  // t1.join();
  // t2.join();

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

  // autotune::queue_thread_pool<8> pool;
  // pool.start();

  // pool.enqueue_work(my_function, 1);
  // pool.enqueue_work(my_function, 2);
  // pool.enqueue_work(my_function, 3);
  // pool.enqueue_work(my_function_without_meta, 666);
  // pool.enqueue_work(my_function, 4);
  // for (size_t i = 0; i < 10; i++) {
  //   pool.enqueue_work(my_function, static_cast<int32_t>(i));
  // }

  // print_mutex.lock();
  // std::cout << "finished queuing work" << std::endl;
  // print_mutex.unlock();

  // pool.finish();

  const size_t N = 4;
  std::vector<double> A(N * N);
  std::fill(A.begin(), A.end(), 1.0);

  std::vector<double> B(N * N);
  std::fill(B.begin(), B.end(), 2.0);
  std::vector<double> C(N * N);
  std::fill(C.begin(), C.end(), 0.0);

  std::cout << "A:" << std::endl;
  print_matrix(A, N);
  std::cout << "B:" << std::endl;
  print_matrix(B, N);

  std::function<void(autotune::thread_meta)> mult_kernel = [&A, &B, &C, &N](
      autotune::thread_meta meta) {
    // size_t base_x = meta.x;
    // size_t base_y = meta.y;
    print_mutex.lock();
    std::cout << "working on (" << meta.x << ", " << meta.y << ")" << std::endl;
    print_mutex.unlock();
    for (size_t i = 0; i < N; i++) {
      C[meta.x * N + meta.y] += A[meta.x * N + i] * B[i * N + meta.y];
    }
  };

  autotune::grid_executor<1> grid_exe;
  autotune::grid_spec spec;
  // span a 2x2 grid, with block sizes 10x10 (overall 20x20)
  spec.grid_z = 1;
  spec.grid_y = 2;
  spec.grid_x = 2;
  spec.block_z = 1;
  spec.block_y = 2;
  spec.block_x = 2;
  grid_exe(mult_kernel, spec);

  std::cout << "C:" << std::endl;
  print_matrix(C, N);
}
