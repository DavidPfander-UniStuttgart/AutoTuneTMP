#pragma once

#include <atomic>
#include <memory>
#include <mutex>

namespace autotune {
namespace detail {

template <typename T> class queue_node {
public:
  T data;
  std::shared_ptr<queue_node<T>> next_node;
};

// stores Ts
template <typename T> class thread_safe_queue {
private:
  std::shared_ptr<queue_node<T>> front_node;
  std::shared_ptr<queue_node<T>> back_node;
  // front node as viewed from outside, for fewer data structure updates
  // (especially involding any locking)
  std::shared_ptr<queue_node<T>> cur_node;

  std::mutex push_mutex;
  std::mutex next_mutex;
  std::mutex reserve_mutex;

  std::atomic<size_t> enqueued_elements;

public:
  thread_safe_queue()
      : enqueued_elements(0) { // , clean_up_frequency(50), next_counter(0)
    auto dummy = std::make_shared<queue_node<T>>();
    // front_node = dummy;
    back_node = dummy;
    cur_node = dummy;
  }

  void push_back(T t) {
    // required in case front_node == back_node
    std::unique_lock<std::mutex> lock(push_mutex);
    auto new_element = std::make_shared<queue_node<T>>();
    new_element->data = std::move(t);
    back_node->next_node = new_element;
    back_node = new_element;
    enqueued_elements += 1;
  }

  // invalidates the current pointed to object in the queue
  // "reserve" needs to be called before "next" is called
  T next() {
    // required as cur_node node and one node is modified
    std::unique_lock<std::mutex> lock(next_mutex);
    // always the case after the first node was inserted
    cur_node = cur_node->next_node;
    // cur_node's data is invalidated, but the node still exists
    return std::move(cur_node->data);
  }

  // should not be called, mostly for debugging
  // enqueuing thread should notfity threads calling "next()"
  bool reserve() {
    std::unique_lock<std::mutex> lock(reserve_mutex);
    if (enqueued_elements > 0) {
      // can only be further increased by "push_back" due to "reserve_mutex"
      enqueued_elements -= 1;
      return true;
    } else {
      return false;
    }
  }
};
}
}
