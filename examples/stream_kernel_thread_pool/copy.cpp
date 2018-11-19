#include "autotune/execution_wrapper.hpp"
#include "autotune/queue_thread_pool.hpp"
#include "parameters.hpp"
#include <boost/align/aligned_allocator.hpp>
#include <chrono>
#include <iostream>
#include <omp.h>
#include <vector>

using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;

#include "opttmp/vectorization/register_tiling.hpp"
using reg_arr =
    opttmp::vectorization::register_array<Vc::double_v, REG_BLOCKING>;
using align = boost::alignment::aligned_allocator<double, 64>;

void print_bandwidth(double bytes_traffic, double duration_s_avr) {
  double bandwidth_gb = (bytes_traffic / duration_s_avr) * 1E-9;
  std::cout << "bandwidth GB/s: " << bandwidth_gb << std::endl;
}

AUTOTUNE_EXPORT void copy(const size_t N_per_task, const size_t repeat) {

  // const size_t N = N_per_task * KERNEL_THREADS;
  std::array<std::vector<double, align>, KERNEL_THREADS> a;
  std::array<std::vector<double, align>, KERNEL_THREADS> b;

  autotune::queue_thread_pool<KERNEL_THREADS> pool;
  pool.start();

  std::function<void(size_t)> f_init = [&](size_t thread_id) {
    a[thread_id].resize(N_per_task);
    for (size_t i = 0; i < N_per_task; i += 1) {
      a[thread_id][i] = 0.0;
    }
    b[thread_id].resize(N_per_task);
    for (size_t i = 0; i < N_per_task; i += 1) {
      b[thread_id][i] = static_cast<double>(i);
    }
  };

  for (size_t i = 0; i < KERNEL_THREADS; i += 1) {
    pool.enqueue_work_id(f_init, static_cast<size_t>(0));
  }

  {
    time_point start = high_resolution_clock::now();
    for (size_t r = 0; r < repeat; r += 1) {
      std::function<void(size_t)> f = [&](size_t thread_id) {
        std::vector<double, align> &a_thread = a[thread_id];
        std::vector<double, align> &b_thread = b[thread_id];
        for (size_t i = 0; i < N_per_task; i += REG_BLOCKING * Vc::double_v::size()) {
          reg_arr temp(&b_thread[i], Vc::flags::vector_aligned);
          temp.memstore(&a_thread[i], Vc::flags::vector_aligned);
        }
      };

      for (size_t i = 0; i < KERNEL_THREADS; i += 1) {
        pool.enqueue_work_id(f, static_cast<size_t>(0));
      }
    }
    pool.finish();
    time_point end = high_resolution_clock::now();
    double duration_s_avr =
        std::chrono::duration<double>(end - start).count() / repeat;
    double bytes_copied = 8.0 * 3.0 * N_per_task * KERNEL_THREADS;
    std::cout << "scale: 2 read, 1 write" << std::endl;
    print_bandwidth(bytes_copied, duration_s_avr);
  }

  //   omp_set_num_threads(KERNEL_OMP_THREADS);
  //   // std::cout << "KERNEL_OMP_THREADS: " << KERNEL_OMP_THREADS <<
  //   std::endl;

  //   const size_t N = a.size();
  // #pragma omp parallel for
  //   for (size_t i = 0; i < N; i += REG_BLOCKING * Vc::double_v::size()) {
  //     reg_arr temp(&b[i], Vc::flags::vector_aligned);
  //     temp.memstore(&a[i], Vc::flags::vector_aligned);
  //   }
}
