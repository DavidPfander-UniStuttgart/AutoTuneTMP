#pragma once

#include <atomic>
#include <condition_variable>
#include <iostream>

// extern std::mutex print_mutex;

namespace autotune {
namespace detail {
template <typename T> class fixed_atomic_queue {
private:
  std::mutex mutex;
  size_t max_elements;
  std::vector<T> queue;
  size_t front_index;
  size_t insert_index;
  size_t count;
  std::condition_variable cv_ready;

public:
  fixed_atomic_queue(size_t max_elements)
      : max_elements(max_elements), queue(max_elements), front_index(0),
        insert_index(0), count(0) {
    std::fill(queue.begin(), queue.end(), 666);
  }
  void push(const T &element) {
    std::unique_lock l(mutex);
    if (count == max_elements) {
      throw; // is full!
    }
    queue[insert_index] = element;
    std::cout << "cur insert_index: " << insert_index << std::endl;
    insert_index = (insert_index + 1) % max_elements;
    std::cout << "new insert_index: " << insert_index << std::endl;
    count += 1;
    cv_ready.notify_one();
  }
  T pop() {
    std::unique_lock l(mutex);
    // std::unique_lock l2(print_mutex);
    std::cout << "queue state: ";
    for (size_t i = 0; i < max_elements; i++) {
      if (i > 0) {
        std::cout << " ";
      }
      std::cout << queue[i];
    }
    std::cout << std::endl;
    std::cout << "front_index:" << front_index
              << " insert_index: " << insert_index << std::endl;

    if (count == 0) {
      throw; // is empty!
    }
    T temp = std::move(queue[front_index]);
    queue[front_index] = 666;
    std::cout << "cur front_index: " << front_index << std::endl;
    front_index = (front_index + 1) % max_elements;
    std::cout << "new front_index: " << front_index << std::endl;
    count -= 1;
    return temp;
  }
  bool peek() {
    // TODO: also only works with only a single host thread
    std::unique_lock l(mutex);
    bool is_empty = count > 0;
    return is_empty;
  }
  // TODO: need multiple host mutices, otherwise other hosts block on
  // unique_lock creation
  void wait_ready(std::mutex &mutex_host) {
    std::unique_lock l(mutex);
    bool has_element = count > 0;
    l.unlock();

    if (has_element) {
      std::cout << "is_empty: false, count: " << count << std::endl;
      return; // TODO: bug, someone can make it empty while I return, have to
              // reserve it
    }
    std::cout << "wait_ready: before wait call" << std::endl;
    std::unique_lock<std::mutex> l_m(mutex_host);
    cv_ready.wait(l_m);
  }
};
}
}
