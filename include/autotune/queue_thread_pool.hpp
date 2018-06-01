#pragma once

#include "execution_wrapper.hpp"
#include "thread_safe_queue.hpp"
#include <boost/thread.hpp>
#include <errno.h>
#include <pthread.h>

#include <condition_variable>
#include <iostream>
#include <thread>

// extern std::mutex print_mutex;

// thread_local size_t thread_id;

// size_t get_thread_id() { return thread_id; }

namespace autotune {

template <size_t num_threads> class queue_thread_pool {
private:
  std::array<std::thread, num_threads> threads;
  std::condition_variable threads_wait_cv;
  // used for threads to wait for work. Also used to lock for finishing
  std::mutex mutex_threads;
  bool threads_finish;

  detail::thread_safe_queue<std::unique_ptr<detail::abstract_executor>> safe_q;

  uint32_t physical_concurrency;

  bool verbose;

  // int32_t count_cpus() {
  //   cpu_set_t cpu_set;
  //   CPU_ZERO(&cpu_set);
  //   // will only count online CPUs
  //   sched_getaffinity(0, sizeof(cpu_set), &cpu_set);

  //   int32_t count = 0;
  //   for (int i = 0; i < CPU_SETSIZE; i++) {
  //     if (CPU_ISSET(i, &cpu_set))
  //       count += 1;
  //   }
  //   return count;
  // }

  void worker_main(size_t i) {

    // set thread affinity to the core of your number
    pthread_t thread = pthread_self();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(static_cast<int>(i % physical_concurrency), &cpuset);
    if (verbose) {
      std::cout << "queue_thread_pool: thread i: " << i
                << " bound to core: " << (i % physical_concurrency)
                << std::endl;
    }

    int s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (s != 0) {
      errno = s;
      perror("pthread_setaffinity_np");
      exit(EXIT_FAILURE);
    }

    while (true) {

      // check for work
      std::unique_ptr<detail::abstract_executor> exe;
      if (safe_q.reserve()) {
        // work found
        exe = std::move(safe_q.next());
      }

      if (exe) {
        exe->set_thread_id(i);

        // if work was found, execute it
        // if (auto *casted =
        //         dynamic_cast<detail::delayed_executor_id *>(exe.get())) {
        //   (*casted)(i);
        // } else {
        (*exe)();
        // }
      } else {
        // no work was found, wait for work or finish signal
        std::unique_lock lock(mutex_threads);
        // spurious wakeups can occur
        // this is not a problem a thread will re-check for work and then
        // continue sleeping
        if (threads_finish) {
          // finish signal received
          break;
        }
        threads_wait_cv.wait(lock); // , [this]() { return threads_finish; }
      }
    }
  }

public:
  int64_t THREAD_ID_PLACEHOLDER = 0;

  queue_thread_pool(bool verbose = false)
      : threads_finish(false),
        physical_concurrency(boost::thread::physical_concurrency()),
        verbose(verbose) {
    // std::cout << "using no. of cpu cores: " << cpu_cores << std::endl;
    // std::cout << "boost physical concurrency: "
    //           << boost::thread::physical_concurrency() << std::endl;
    // std::cout << "boost hardware concurrency: "
    //           << boost::thread::hardware_concurrency() << std::endl;
    if (verbose) {
      std::cout << "queue_thread_pool: physical_concurrency: "
                << physical_concurrency << std::endl;
    }
    if (num_threads > physical_concurrency) {
      std::cout << "queue_thread_pool: warning: using more threads than "
                   "available cores"
                << std::endl;
    }
  } // next_work(0), last_work(0)

  // creates and starts threads of thread pool (non-blocking)
  void start() {
    // TODO: how to the reset the class? cv variable? host_finish?
    for (size_t i = 0; i < num_threads; i++) {
      threads[i] =
          std::thread(&queue_thread_pool<num_threads>::worker_main, this, i);
    }
  }

  // trigger shutting down the thread pool (blocking), waits for all remaining
  // work to be processed
  void finish() {
    // lock to prevent racecondition between "threads_finish" and subsequent
    // wait
    mutex_threads.lock();
    threads_finish = true;
    threads_wait_cv.notify_all();
    // need to unlock for the join to work
    mutex_threads.unlock();
    for (size_t i = 0; i < num_threads; i++) {
      threads[i].join();
    }
  }

  // enqueue arbitrary function with void return, arguments have to be copyable
  template <typename... Args>
  void enqueue_work(std::function<void(Args...)> f, Args... args) {
    auto work_temp = std::unique_ptr<detail::abstract_executor>(
        new detail::delayed_executor<Args...>(f, std::move(args)...));
    safe_q.push_back(std::move(work_temp));
    threads_wait_cv.notify_one();
  }

  // enqueue arbitrary function with void return, arguments have to be copyable
  template <typename... Args>
  void enqueue_work_id(std::function<void(Args...)> f, Args... args) {
    auto work_temp = std::unique_ptr<detail::abstract_executor>(
        new detail::delayed_executor_id<Args...>(f, std::move(args)...));
    safe_q.push_back(std::move(work_temp));
    threads_wait_cv.notify_one();
  }

  // // // enqueue function with void return that accepts thread metadata as its
  // first
  // // // argument. Arguments have to be copyable
  // // template <typename... Args>
  // // void enqueue_work(std::function<void(thread_meta, Args...)> f, Args...
  // // args) {
  // //   auto work_temp = std::unique_ptr<detail::abstract_executor>(
  // //       new detail::delayed_executor_meta<Args...>(f, args...));
  // //   // print_mutex.lock();
  // //   // std::cout << "enqueue work with meta (holding mutex)" << std::endl;
  // //   // print_mutex.unlock();
  // //   safe_q.push_back(std::move(work_temp));
  // //   threads_wait_cv.notify_one();
  // //   // print_mutex.lock();
  // //   // std::cout << "ONE thread notified" << std::endl;
  // //   // std::cout << "enqueue work with meta (releasing mutex)" <<
  // std::endl;
  // //   // print_mutex.unlock();
  // // }
};
} // namespace autotune
