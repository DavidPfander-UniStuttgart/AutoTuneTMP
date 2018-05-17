#pragma once

#include "execution_wrapper.hpp"
#include "thread_safe_queue.hpp"

#include <condition_variable>
#include <iostream>
#include <thread>

// extern std::mutex print_mutex;

namespace autotune {

template <size_t num_threads> class queue_thread_pool {
private:
  std::array<std::thread, num_threads> threads;
  std::condition_variable threads_wait_cv;
  // used for threads to wait for work. Also used to lock for finishing
  std::mutex mutex_threads;
  bool threads_finish;

  detail::thread_safe_queue<std::unique_ptr<detail::abstract_executor>> safe_q;

  void worker_main(size_t i) {

    while (true) {

      // check for work
      std::unique_ptr<detail::abstract_executor> exe;
      // print_mutex.lock();
      // std::cout << "look for work (waiting for mutex)" << std::endl;
      // print_mutex.unlock();
      // mutex_threads.lock();
      // print_mutex.lock();
      // std::cout << "look for work (holding mutex)" << std::endl;
      // print_mutex.unlock();
      if (safe_q.reserve()) {
        // work found
        exe = std::move(safe_q.next());
      }
      // print_mutex.lock();
      // std::cout << "look for work (releasing mutex)" << std::endl;
      // print_mutex.unlock();
      // mutex_threads.unlock();

      if (exe) {
        // if work was found, execute it
        thread_meta meta;
        meta.x = i;
        meta.y = 0;
        meta.z = 0;
        exe->set_meta(meta);
	// std::cout << "doing work in t: " << i << std::endl;
        (*exe)();
      } else {
        // no work was found, wait for work or finish signal
        std::unique_lock lock(mutex_threads);
        // spurious wakeups can occur
        // this is not a problem a thread will re-check for work and then
        // continue sleeping
        if (threads_finish) {
          // finish signal received
          // print_mutex.lock();
          // std::cout << "thread now finishing (break)" << std::endl;
          // print_mutex.unlock();
          // mutex_work.unlock();
          break;
        }
        // print_mutex.lock();
        // std::cout << "thread: " << i << " now waiting" << std::endl;
        // print_mutex.unlock();
        threads_wait_cv.wait(lock); // , [this]() { return threads_finish; }
        // print_mutex.lock();
        // std::cout << "thread: " << i << " woke up" << std::endl;
        // print_mutex.unlock();
      }
    }
    // print_mutex.lock();
    // std::cout << "thread: " << i << " -> finished" << std::endl;
    // print_mutex.unlock();
  }

public:
  queue_thread_pool() : threads_finish(false) {} // next_work(0), last_work(0)

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
    // print_mutex.lock();
    // std::cout << "before aquiring lock to finish" << std::endl;
    // print_mutex.unlock();
    mutex_threads.lock();
    threads_finish = true;
    threads_wait_cv.notify_all();
    // need to unlock for the join to work
    mutex_threads.unlock();
    // print_mutex.lock();
    // std::cout << "all threads notified" << std::endl;
    // print_mutex.unlock();
    for (size_t i = 0; i < num_threads; i++) {
      threads[i].join();
    }
  }

  // enqueue arbitrary function with void return, arguments have to be copyable
  template <typename... Args>
  void enqueue_work(std::function<void(Args...)> f, Args... args) {
    auto work_temp = std::unique_ptr<detail::abstract_executor>(
        new detail::delayed_executor<Args...>(f, std::move(args)...));
    // print_mutex.lock();
    // std::cout << "enqueue work (holding mutex)" << std::endl;
    // print_mutex.unlock();
    safe_q.push_back(std::move(work_temp));
    threads_wait_cv.notify_one();
    // print_mutex.lock();
    // std::cout << "ONE thread notified" << std::endl;
    // std::cout << "enqueue work (releasing mutex)" << std::endl;
    // print_mutex.unlock();
  }

  // // // enqueue function with void return that accepts thread metadata as its first
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
  // //   // std::cout << "enqueue work with meta (releasing mutex)" << std::endl;
  // //   // print_mutex.unlock();
  // // }
};
}
