#pragma once

#include <list>
#include <thread>

#include "execution_wrapper.hpp"
#include "fixed_atomic_queue.hpp"

extern std::mutex print_mutex;

namespace autotune {

template <size_t num_threads> class simple_thread_pool {
private:
  std::array<std::thread, num_threads> threads;
  // stores work to be performed by a working thread in the future
  std::list<std::unique_ptr<detail::abstract_executor>> work_list;
  // lock whenever the work_list is modified
  std::mutex mutex_work;

  std::condition_variable threads_wait_cv;
  bool threads_finish;

  void worker_main(size_t i) {
    std::mutex mtx_thread;
    while (true) {

      // check for work
      std::unique_ptr<detail::abstract_executor> exe;
      if (work_list.size() > 0) {
        // work found
        mutex_work.lock();
        exe = std::move(work_list.front());
        work_list.pop_front();
        mutex_work.unlock();
      } else if (threads_finish) {
        // finish signal received
        break;
      }

      if (exe) {
        // if work was found, execute it
        thread_meta meta;
        meta.x = i;
        meta.y = 0;
        meta.z = 0;
        exe->set_meta(meta);
        (*exe)();
      } else {
        // no work was found, wait for work or finish signal
        // TODO: likely there is a race condition in here somewhere
        std::unique_lock lock(mtx_thread);
        threads_wait_cv.wait(lock);
      }
    }
    print_mutex.lock();
    std::cout << "thread: " << i << " -> finished" << std::endl;
    print_mutex.unlock();
  }

public:
  simple_thread_pool() : threads_finish(false) {}

  // creates and starts threads of thread pool (non-blocking)
  void start() {
    // TODO: how to the reset the class? cv variable? host_finish?

    for (size_t i = 0; i < num_threads; i++) {
      threads[i] =
          std::thread(&simple_thread_pool<num_threads>::worker_main, this, i);
    }
  }

  // trigger shutting down the thread pool (blocking), waits for all remaining
  // work to be processed
  void finish() {
    threads_finish = true;
    threads_wait_cv.notify_all();
    for (size_t i = 0; i < num_threads; i++) {
      threads[i].join();
    }
  }

  // enqueue arbitrary function with void return, arguments have to be copyable
  template <typename... Args>
  void enqueue_work(std::function<void(Args...)> f, Args... args) {
    auto work_temp = std::unique_ptr<detail::abstract_executor>(
        new detail::delayed_executor<Args...>(f, args...));
    std::unique_lock<std::mutex> lock(mutex_work);
    work_list.push_back(std::move(work_temp));
    threads_wait_cv.notify_one();
  }

  // enqueue function with void return that accepts thread metadata as its first
  // argument. Arguments have to be copyable
  template <typename... Args>
  void enqueue_work(std::function<void(thread_meta, Args...)> f, Args... args) {
    auto work_temp = std::unique_ptr<detail::abstract_executor>(
        new detail::delayed_executor_meta<Args...>(f, args...));
    std::unique_lock<std::mutex> lock(mutex_work);
    work_list.push_back(std::move(work_temp));
    threads_wait_cv.notify_one();
  }
};
}
