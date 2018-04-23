#include "autotune/thread_pool.hpp"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>

// // template <size_t... indices> void actor(std::index_sequence<indices...>)
// {}
// // }

// template <size_t num_threads, typename R, typename... Args> struct executor {
//   std::function<R(Args...)> functor;
//   std::thread t;
//   std::mutex mtx;
//   std::condition_variable cv;
//   volatile bool ready;
//   bool work_available;
//   std::tuple<Args...> work;
//   // constexpr args_num = sizeof...(Args);
//   // std::array<std::thread, num_threads> threads;
//   // std::array<std::condition_variable, num_threads> busy;
//   // std::array<std::condition_variable, num_threads> has_work;
//   template <typename F> executor(F f) : functor(f), work_available(false) {

//     // t = std::thread([this]() {
//     //   // std::unique_lock<std::mutex> lock(mtx);
//     //   // if (!ready) // in case the notification was in the past
//     //   //   // alternative: busy wait on ready, but uses up a CPU
//     //   //   std::cout << "not ready, will now wait" << std::endl;
//     //   // cv.wait(lock);
//     // });
//     auto wrapper = [this](size_t... indices) {
//       static_assert(sizeof...(Args) == 1);
//       t = std::thread([this, indices...]() {
//         while (true) {
//           if (work_available) {
//             // TODO: add mutex on work (for other threads)
//             std::tuple<Args...> temp = std::move(work);
//             work_available = false;
//             functor(std::get<indices>(temp)...);
//           }
//         }
//       }); // std::make_index_sequence<sizeof...(Args)>{}
//           // //&executor<num_threads, R, Args...>::actor<indices...>
//     };
//     wrapper(std::make_index_sequence<sizeof...(Args)>{});
//   }

//   // template <size_t... indices> void actor() {}

//   void invoke_async(Args... args) {
//     ready = false;

//     std::this_thread::sleep_for(std::chrono::seconds(3));
//     ready = true;
//     cv.notify_all();
//     std::cout << "notified! yeah!" << std::endl;
//   }

//   void join() {

//     // std::for_each(threads.begin(), threads.end(),
//     //               [](std::thread &t) { t.join(); });
//     t.join();
//   }
// };

// int main(void) {
//   // autotune::thread_pool<2> pool;
//   auto f = [](int blubb) {
//     std::cout << blubb * 2 << std::endl;

//   };

//   executor<1, void, int> e(f);
//   e.invoke_async(2);
//   // e.invoke_async(3);
//   // e.invoke_async(4);
//   e.join();

//   // std::thread t(f, 2);
//   // t.join();
// }

std::mutex print_mutex;

void kernel(int a, int b) {
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::unique_lock l(print_mutex);
  std::cout << (b + a) << std::endl;
}

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
    std::unique_lock l2(print_mutex);
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

int a = 2;
int b = 1;

class abstract_executor {
public:
  virtual void operator()() = 0;
};

template <typename... Args> class delayed_executor : abstract_executor {
private:
  std::function<void(Args...)> f;
  std::tuple<Args...> copied_arguments;

  template <size_t... indices>
  void tuple_unwrapped(std::index_sequence<indices...>) {
    f(std::get<indices>(std::move(copied_arguments))...);
  }

public:
  delayed_executor(std::function<void(Args...)> f, Args... args)
      : f(f), copied_arguments(std::move(args)...) {}

  virtual void operator()() override {
    tuple_unwrapped(std::make_index_sequence<sizeof...(Args)>{});
  }
};

template <size_t num_threads> class simple_thread_pool {
private:
  // signal that are thread is ready for new work (used together with
  // thread_ready)
  std::array<std::condition_variable, num_threads> cv_thread;
  // signal that are thread is ready for new work (used together with cv_thread)
  std::array<bool, num_threads> thread_ready;
  // used to signal that threads can finish
  std::array<bool, num_threads> thread_finish;
  fixed_atomic_queue<size_t> q; // stores indices of threads that are ready
  std::vector<std::thread> threads;
  // used to singal the host that new work is to be distributed
  std::mutex mutex_host;
  // stores work to be performed by a working thread in the future
  std::list<std::unique_ptr<abstract_executor>> work_list;
  std::array<std::unique_ptr<abstract_executor>, num_threads> assigned_work;
  // lock whenever the work_list is modified
  std::mutex mutex_work;

  bool host_finish = false;

  void worker_main(size_t i) {
    std::mutex mtx_thread;
    while (true) {
      // wait for ready signal from host thread
      // condition variable + signal used to avoid spin lock
      if (!thread_ready[i]) {
        std::unique_lock<std::mutex> lock(mtx_thread);
        cv_thread[i].wait(lock);
      }
      // signal received, no longer ready
      // update of thread ready can be delayed, as thread is no longer
      // enqueued
      thread_ready[i] = false;
      // check for finishing up, else execute kernel
      if (thread_finish[i]) {
        print_mutex.lock();
        std::cout << "thread: " << i << " -> got finish signal, breaking"
                  << std::endl;
        print_mutex.unlock();
        break;
      } else {
        print_mutex.lock();
        std::cout << "thread: " << i << " -> executes kernel" << std::endl;
        print_mutex.unlock();

        (*(assigned_work[i]))();
      }
      // advertise that the thread now idles
      print_mutex.lock();
      std::cout << "thread: " << i << " -> now enqueing again" << std::endl;
      print_mutex.unlock();
      q.push(i); // notifies host in the end
    }
    print_mutex.lock();
    std::cout << "thread: " << i << " -> finished" << std::endl;
    print_mutex.unlock();
  }

public:
  simple_thread_pool() : q(num_threads) {
    std::fill(thread_ready.begin(), thread_ready.end(), true);
    std::fill(thread_finish.begin(), thread_finish.end(), false);
  }
  void start() {
    // TODO: how to the reset the class? cv variable? host_finish?

    for (size_t i = 0; i < num_threads; i++) {
      threads.emplace_back(&simple_thread_pool<num_threads>::worker_main, this,
                           i);
    }

    // print_mutex.lock();
    // std::cout << "host: starting to schedule work" << std::endl;
    // print_mutex.unlock();

    // while (!host_finish) {
    //   print_mutex.lock();
    //   std::cout << "before wait_ready" << std::endl;
    //   print_mutex.unlock();
    //   q.wait_ready(mutex_host);
    //   // removes thread from queue, but not yet signaled that work is ready
    //   print_mutex.lock();
    //   std::cout << "thread ready, popping" << std::endl;
    //   print_mutex.unlock();

    //   // which thread is ready?
    //   size_t next_worker = q.pop();

    //   // configure work here
    //   mutex_work.lock();
    //   std::unique_ptr<abstract_executor> worker_exe(
    //       std::move(work_list.front()));
    //   work_list.pop_front();
    //   mutex_work.unlock();
    //   assigned_work[next_worker] = std::move(worker_exe);

    //   print_mutex.lock();
    //   std::cout << "host: thread '" << next_worker << "' got work assigned"
    //             << std::endl;
    //   print_mutex.unlock();
    //   cv_thread[next_worker].notify_all(); // TODO: verify order!
    //   thread_ready[next_worker] = true;
    // }

    // print_mutex.lock();
    // std::cout << "host: sending finished signal to all threads" << std::endl;
    // print_mutex.unlock();
    // size_t finished_threads = 0;
    // while (finished_threads < num_threads) {
    //   // wait for thread become ready for new work
    //   while (!q.peek()) {
    //   }
    //   // removes thread from queue, but not yet signaled that work is ready
    //   size_t next_worker = q.pop();
    //   thread_finish[next_worker] = true;
    //   cv_thread[next_worker].notify_all(); // TODO: verify order!
    //   thread_ready[next_worker] = true;
    //   std::cout << "host: signal sent to thread: " << next_worker << std::endl;
    //   finished_threads += 1;
    // }
    // print_mutex.lock();
    // std::cout << "host: now joining" << std::endl;
    // print_mutex.unlock();

    // for (size_t i = 0; i < num_threads; i++) {
    //   print_mutex.lock();
    //   std::cout << "host: joining thread: " << i << std::endl;
    //   print_mutex.unlock();
    //   threads[i].join();
    // }
    // print_mutex.lock();
    // std::cout << "host: done joining" << std::endl;
    // print_mutex.unlock();
  }

  void stop() { host_finish = true; }

  template <typename... Args>
  void enqueue_work(std::function<void(Args...)> f, Args... args) {
    std::unique_lock<std::mutex> lock(mutex_work);
    work_list.push_back(
        std::move(std::make_unique<delayed_executor>(f, args...)));
  }
};

int main(void) {

  std::function<void(int32_t)> my_function = [](int32_t a) {
    std::cout << "hello from my_function: " << a << std::endl;
  };

  delayed_executor exe(my_function, 3);

  exe();

  // simple_thread_pool<2> pool;
  // pool.start();
}
