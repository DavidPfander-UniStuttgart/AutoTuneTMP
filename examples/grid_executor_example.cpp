#include "autotune/grid_executor.hpp"
#include "autotune/execution_wrapper.hpp"
#include "autotune/queue_thread_pool.hpp"
#include "autotune/thread_pool.hpp"
#include "autotune/thread_safe_queue.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>

#include <Vc/Vc>
using Vc::double_v;

std::mutex print_mutex;

int a = 2;

template <typename T> void print_matrix(std::vector<T> &m, size_t N) {
  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < N; j++) {
      if (j > 0)
        std::cout << ", ";
      std::cout << m[i * N + j];
    }
    std::cout << std::endl;
  }
}

template <typename T, typename U>
void naive_matrix_multiplication(std::vector<T> &A, std::vector<T> &B,
                                 std::vector<U> &C, size_t N) {
  for (size_t x = 0; x < N; x++) {
    for (size_t y = 0; y < N; y++) {
      for (size_t k = 0; k < N; k++) {
        C[x * N + y] += A[x * N + k] * B[k * N + y];
      }
    }
  }
}

template <typename T, typename U>
bool compare_matrices(std::vector<T> &m, std::vector<U> &n, size_t N) {
  for (size_t x = 0; x < N; x++) {
    for (size_t y = 0; y < N; y++) {
      if (m[x * N + y] - n[x * N + y] >= 1E-9) {
        // throw "matrix not equal";
        std::cout << "error: " << m[x * N + y] << " != " << n[x * N + y]
                  << std::endl;
        return false;
      }
    }
  }
  std::cout << "matrices are equal" << std::endl;
  return true;
}

template <typename T>
void add_atomically(std::vector<T> &m, size_t base_index,
                    double_v value_to_add) {
  for (size_t i = 0; i < double_v::size(); i++) {
    // vector width many atomic updates
    while (true) {
      double org = m[base_index + i];
      double new_val = org + value_to_add[i];
      if (m[base_index + i].compare_exchange_weak(org, new_val)) {
        break;
      }
    }
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

  const size_t N = 2048;
  const size_t z_block_size = 64;
  const size_t x_y_block_size = 16; // 256
  bool compare_with_naive = false;
  if (N < double_v::size()) {
    throw "matrix too small for configured vector width, make \"N\" larger!";
  }
  if (z_block_size < double_v::size()) {
    throw "\"z_block_size\" too small for configured vector width, make "
          "\"z_block_size\" larger!";
  }
  if (z_block_size < double_v::size()) {
    throw "\"x_y_block_size\" too small for configured vector width, make "
          "\"x_y_block_size\" larger!";
  }

  std::vector<double> A(N * N);
  double fillup = 0.0;
  std::generate(A.begin(), A.end(), [&fillup]() {
    fillup += 1.0;
    return fillup;
  });

  std::vector<double> B(N * N);
  fillup = 0.5;
  std::fill(B.begin(), B.end(), 1.0);
  // std::generate(B.begin(), B.end(), [&fillup]() {
  //   fillup += 1;
  //   return fillup;
  // });
  std::vector<std::atomic<double>> C(N * N);
  std::fill(C.begin(), C.end(), 0.0);

  // std::cout << "A:" << std::endl;
  // print_matrix(A, N);
  // std::cout << "B:" << std::endl;
  // print_matrix(B, N);

  std::function<void(autotune::thread_meta)> mult_kernel =
      [&A, &B, &C,
       &N, // &result_mutex
       z_block_size](autotune::thread_meta meta) {
        double_v C_comp(0.0);
        size_t z_base = meta.z * z_block_size;
        for (size_t i = z_base; i < z_base + z_block_size; i++) {
          double_v A_comp(A[meta.y * N + i]);
          double_v B_comp(&B[i * N + meta.x], Vc::flags::element_aligned);
          C_comp += A_comp * B_comp;
        }

        add_atomically(C, meta.y * N + meta.x, C_comp);
      };

  autotune::grid_executor<8, double_v::size()> grid_exe;
  autotune::grid_spec spec;
  // span a 3d grid
  spec.block_z = 1;
  spec.block_y = x_y_block_size;
  spec.block_x = x_y_block_size;
  // spec.grid_z = 1;
  spec.grid_z = N / z_block_size;
  spec.grid_y = N / spec.block_y;
  spec.grid_x = N / spec.block_x;

  std::chrono::high_resolution_clock::time_point start_stamp =
      std::chrono::high_resolution_clock::now();
  grid_exe(mult_kernel, spec);
  std::chrono::high_resolution_clock::time_point stop_stamp =
      std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> time_span =
      std::chrono::duration_cast<std::chrono::duration<double>>(stop_stamp -
                                                                start_stamp);
  double duration = time_span.count();

  double flop = 2 * N * N * N * 1E-9;
  std::cout << "Gflop: " << flop << std::endl;
  std::cout << "duration: " << duration << "s" << std::endl;
  std::cout << "Gflop/s: " << (flop / duration) << std::endl;

  if (compare_with_naive) {
    std::vector<double> C_ref(N * N);
    std::fill(C_ref.begin(), C_ref.end(), 0.0);

    std::chrono::high_resolution_clock::time_point start_stamp_naive =
        std::chrono::high_resolution_clock::now();
    naive_matrix_multiplication(A, B, C_ref, N);
    std::chrono::high_resolution_clock::time_point stop_stamp_naive =
        std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span_naive =
        std::chrono::duration_cast<std::chrono::duration<double>>(
            stop_stamp_naive - start_stamp_naive);
    double duration_naive = time_span_naive.count();
    compare_matrices(C, C_ref, N);
    std::cout << "duration naive: " << duration_naive << "s" << std::endl;
    std::cout << "Gflop/s naive: " << (flop / duration_naive) << std::endl;
  }

  // std::cout << "C:" << std::endl;
  // print_matrix(C, N);
}
