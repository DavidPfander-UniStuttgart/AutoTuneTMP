#include "autotune/autotune.hpp"
#include "autotune/autotune_exception.hpp"
#include "autotune/execution_wrapper.hpp"
#include "autotune/generalized_kernel.hpp"
#include "autotune/queue_thread_pool.hpp"
#include "autotune/thread_pool.hpp"
#include "autotune/thread_safe_queue.hpp"
#include "autotune/tuned_grid_executor.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>
#include <vector>

#include <Vc/Vc>
using Vc::double_v;

AUTOTUNE_GENERALIZED_KERNEL(void(std::vector<double> &, std::vector<double> &,
                                 std::vector<std::atomic<double>> &, size_t, size_t),
                            grid_mult_kernel)

namespace autotune {
thread_local thread_meta meta;
thread_meta get_meta() { return meta; }

void set_meta(thread_meta meta) { autotune::meta = meta; }
}  // namespace autotuneo

template <typename T>
void add_atomically(std::vector<T> &m, size_t base_index, double_v value_to_add) {
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

template <typename T>
void print_matrix(std::vector<T> &m, size_t N) {
  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < N; j++) {
      if (j > 0) std::cout << ", ";
      std::cout << m[i * N + j];
    }
    std::cout << std::endl;
  }
}

template <typename T, typename U>
void naive_matrix_multiplication(std::vector<T> &A, std::vector<T> &B, std::vector<U> &C,
                                 size_t N) {
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
        std::cout << "error: " << m[x * N + y] << " != " << n[x * N + y] << std::endl;
        return false;
      }
    }
  }
  std::cout << "matrices are equal" << std::endl;
  return true;
}

int main(void) {
  autotune::grid_mult_kernel.set_verbose(true);
  std::cout << "info: vector size is: " << double_v::size() << std::endl;

  size_t N = 4;
  size_t z_block_size = 1;    // 64
  size_t x_y_block_size = 4;  // 16
  bool compare_with_naive = true;
  if (N < double_v::size()) {
    throw "matrix too small for configured vector width, make \"N\" larger!";
  }
  if (x_y_block_size % double_v::size() != 0) {
    throw autotune::autotune_exception("\"x_y_block_size\" does not divide vector size!");
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

  autotune::grid_mult_kernel.set_kernel_functor([](std::vector<double> &A, std::vector<double> &B,
                                                   std::vector<std::atomic<double>> &C, size_t N,
                                                   size_t z_block_size) {
    const autotune::thread_meta &meta = autotune::get_meta();
    std::cout << "meta.z: " << meta.z << std::endl;
    std::cout << "meta.y: " << meta.y << std::endl;
    std::cout << "meta.x: " << meta.x << std::endl;

    // std::cout << "info: vector size IN KERNEL is: " << double_v::size()
    //           << std::endl;

    double_v C_comp(0.0);
    size_t z_base = meta.z * z_block_size;

    for (size_t i = z_base; i < z_base + z_block_size; i++) {
      double_v A_comp(A[meta.y * N + i]);
      double_v B_comp(&B[i * N + meta.x], Vc::flags::element_aligned);
      C_comp += A_comp * B_comp;
    }
    add_atomically(C, meta.y * N + meta.x, C_comp);
  });

  // autotune::grid_mult_kernel.get_builder<cppjit::builder::gcc>().set_cpp_flags(
  //     "-std=c++17 -march=native -mtune=native -g ");
  // autotune::grid_mult_kernel.get_builder<cppjit::builder::gcc>().set_link_flags(
  //     "-std=c++17 -g ");
  // autotune::grid_mult_kernel.get_builder<cppjit::builder::gcc>()
  //     .set_include_paths("-IVc_install/include");

  autotune::grid_spec spec;
  // span a 3d grid
  spec.block_z = 1;
  spec.block_y = x_y_block_size;
  spec.block_x = x_y_block_size;
  // spec.grid_z = 1;
  spec.grid_z = N / z_block_size;
  spec.grid_y = N / spec.block_y;
  spec.grid_x = N / spec.block_x;

  autotune::countable_set parameters;

  // TODO: increase threads!!!
  autotune::tuned_grid_executor<2, double_v::size(), std::vector<double> &, std::vector<double> &,
                                std::vector<std::atomic<double>> &, size_t, size_t>
      tuned_grid_exe(autotune::grid_mult_kernel, spec, parameters);

  std::chrono::high_resolution_clock::time_point start_stamp =
      std::chrono::high_resolution_clock::now();
  tuned_grid_exe(A, B, C, N, z_block_size);
  std::chrono::high_resolution_clock::time_point stop_stamp =
      std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> time_span =
      std::chrono::duration_cast<std::chrono::duration<double>>(stop_stamp - start_stamp);
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
        std::chrono::duration_cast<std::chrono::duration<double>>(stop_stamp_naive -
                                                                  start_stamp_naive);
    double duration_naive = time_span_naive.count();
    compare_matrices(C, C_ref, N);
    std::cout << "duration naive: " << duration_naive << "s" << std::endl;
    std::cout << "Gflop/s naive: " << (flop / duration_naive) << std::endl;
  }

  std::cout << "C:" << std::endl;
  print_matrix(C, N);
}
