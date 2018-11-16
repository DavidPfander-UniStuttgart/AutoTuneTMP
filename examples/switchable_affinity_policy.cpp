#include "autotune/execution_wrapper.hpp"
#include "autotune/queue_thread_pool.hpp"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>
#include <sched.h>
#include <thread>

#include "opttmp/bits.hpp"
#include "opttmp/numa_topology.hpp"

int main(void) {
  // std::function<void(int)> my_function = [](int a) {
  //   using namespace std::chrono_literals;
  //   // std::unique_lock<std::mutex> lock(print_mutex);
  //   // std::cout << "meta.x: " << meta.x << std::endl;
  //   std::cout << "hello from my_function: " << a << std::endl << std::flush;
  //   // std::this_thread::sleep_for(2s);
  // };

  // std::function<void(int)> my_function_without_meta = [](int a) {
  //   // std::unique_lock<std::mutex> lock(print_mutex);
  //   std::cout << "without meta" << std::endl;
  //   std::cout << "hello from my_function: " << a << std::endl;
  // };

  // // autotune::detail::delayed_executor exe(my_function_without_meta, 3);
  // // exe();

  // autotune::queue_thread_pool<4> pool;
  // pool.start();

  // pool.enqueue_work(my_function, 1);
  // pool.enqueue_work(my_function, 2);
  // pool.enqueue_work(my_function, 3);
  // pool.enqueue_work(my_function_without_meta, 666);
  // pool.enqueue_work(my_function, 4);
  // pool.enqueue_work(my_function, 5);

  // pool.finish();

  uint64_t field = 0;
  opttmp::bits::set_bit(field, 1);
  opttmp::bits::set_bit(field, 3);
  opttmp::bits::set_bit(field, 5);

  uint64_t r = opttmp::bits::nthset_bit(field, 1);
  std::cout << "field: " << field << std::endl;
  opttmp::bits::print_bits(field);
  std::cout << std::endl;
  std::cout << "r: " << r << std::endl;
  opttmp::bits::print_bits(r);
  std::cout << std::endl;
  std::cout << "ffs(" << r << "): " << opttmp::bits::ffs_0(r) << std::endl;

  opttmp::numa_topology_t numa_topology;

  numa_topology.print();

  auto cpu_set = numa_topology.get_cpuset_compact(2);

  std::cout << "cpu_set compact: " << std::endl;
  for (size_t i = 0; i < 8; i += 1) {
    if (CPU_ISSET(i, cpu_set.get())) {
      std::cout << "1";
    } else {
      std::cout << "0";
    }
  }
  std::cout << std::endl;

  cpu_set = numa_topology.get_cpuset_sparse(2);

  std::cout << "cpu_set sparse: " << std::endl;
  for (size_t i = 0; i < 8; i += 1) {
    if (CPU_ISSET(i, cpu_set.get())) {
      std::cout << "1";
    } else {
      std::cout << "0";
    }
  }
  std::cout << std::endl;
}
