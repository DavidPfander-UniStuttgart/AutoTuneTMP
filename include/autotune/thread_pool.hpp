#pragma once

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <list>
#include <mutex>
#include <thread>

#include "execution_wrapper.hpp"

extern std::mutex print_mutex;

namespace autotune {

namespace detail {

template <typename T> class queue_node {
public:
  T data;
  std::unique_ptr<queue_node<T>> next_node;
  std::unique_ptr<queue_node<T>> previous_node;
};

// stores Ts
template <typename T> class thread_safe_queue {
private:
  std::shared_ptr<queue_node<T>> front_node;
  std::shared_ptr<queue_node<T>> back_node;
  // front node as viewed from outside, for fewer data structure updates
  // (especially involding any locking)
  std::shared_ptr<queue_node<T>> cur_node;

  std::mutex mutex;

  size_t clean_up_frequency = 50;
  size_t next_counter;

public:
  thread_safe_queue() : next_counter(0) {}

  void push_back(T &&t) {
    // required in case front_node == back_node
    std::unique_lock<std::mutex> lock(mutex);
    auto new_element = std::make_shared<queue_node<T>>(std::move(t));
    if (back_node != nullptr) {
      back_node->next_node = new_element;
      new_element->previous_node = new_element;
    }
    back_node = new_element;
    if (front_node == nullptr) {
      front_node = back_node;
    }
  }

  // invalidates the current pointed to object in the queue
  // TODO: assumption: work is available (guaranteed externally!)
  T next() {
    // required as cur_node node and one node is modified
    std::unique_lock<std::mutex> lock(mutex);
    if (cur_node == nullptr) {
      // happens when the very first node is inserted
      cur_node = front_node;
    } else {
      // always the case after the first node was inserted
      cur_node = cur_node->next_node;
    }
    next_counter += 1;
    if (next_counter == clean_up_frequency) {
      // this cleans up all nodes between the old front_node and the
      // current cur_node/ new front_node
      front_node = cur_node;
      next_counter = 0;
    }
    // cur_node's data is invalidated, but the node still exists
    return std::move(cur_node->data);
  }
};
}

template <size_t num_threads> class simple_thread_pool {
private:
  std::array<std::thread, num_threads> threads;
  // stores work to be performed by a working thread in the future
  std::list<std::unique_ptr<detail::abstract_executor>> work_list;
  // lock whenever the work_list is modified
  // std::mutex mutex_work;

  std::condition_variable threads_wait_cv;
  // used for threads to wait for work. Also used to lock for finishing
  std::mutex mutex_threads;
  // std::mutex mutex_threads_finish;
  bool threads_finish;
  std::atomic<size_t> next_work;
  std::atomic<size_t> last_work;

  void worker_main(size_t i) {

    while (true) {

      // check for work
      std::unique_ptr<detail::abstract_executor> exe;
      print_mutex.lock();
      std::cout << "look for work (waiting for mutex)" << std::endl;
      print_mutex.unlock();
      mutex_threads.lock();
      print_mutex.lock();
      std::cout << "look for work (holding mutex)" << std::endl;
      print_mutex.unlock();
      if (work_list.size() > 0) {
        // work found
        // std::unique_lock<std::mutex> lock(mutex_work);
        exe = std::move(work_list.front());
        work_list.pop_front();
      }
      print_mutex.lock();
      std::cout << "look for work (releasing mutex)" << std::endl;
      print_mutex.unlock();
      mutex_threads.unlock();

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
  simple_thread_pool() : threads_finish(false), next_work(0), last_work(0) {}

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
    // lock to prevent racecondition between "threads_finish" and subsequent
    // wait
    // print_mutex.lock();
    // std::cout << "before aquiring lock to finish" << std::endl;
    // print_mutex.unlock();
    // std::unique_lock<std::mutex> lock(mutex_threads);
    mutex_threads.lock();
    threads_finish = true;
    threads_wait_cv.notify_all();
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
        new detail::delayed_executor<Args...>(f, args...));
    std::unique_lock<std::mutex> lock(mutex_threads);
    // print_mutex.lock();
    // std::cout << "enqueue work (holding mutex)" << std::endl;
    // print_mutex.unlock();
    work_list.push_back(std::move(work_temp));
    threads_wait_cv.notify_one();
    // print_mutex.lock();
    // std::cout << "ONE thread notified" << std::endl;
    // std::cout << "enqueue work (releasing mutex)" << std::endl;
    // print_mutex.unlock();
  }

  // enqueue function with void return that accepts thread metadata as its first
  // argument. Arguments have to be copyable
  template <typename... Args>
  void enqueue_work(std::function<void(thread_meta, Args...)> f, Args... args) {
    auto work_temp = std::unique_ptr<detail::abstract_executor>(
        new detail::delayed_executor_meta<Args...>(f, args...));
    std::unique_lock<std::mutex> lock(mutex_threads);
    print_mutex.lock();
    std::cout << "enqueue work with meta (holding mutex)" << std::endl;
    print_mutex.unlock();
    work_list.push_back(std::move(work_temp));
    threads_wait_cv.notify_one();
    print_mutex.lock();
    std::cout << "ONE thread notified" << std::endl;
    std::cout << "enqueue work with meta (releasing mutex)" << std::endl;
    print_mutex.unlock();
  }

  void set_cleanup_frequency(size_t clean_up_frequency) {
    this->clean_up_frequency = clean_up_frequency;
  }
};
}
