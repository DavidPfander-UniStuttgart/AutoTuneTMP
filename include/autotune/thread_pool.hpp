#pragma once

#include <thread>

namespace autotune {

// template <typename R, typename... Args> class trivial_executor {

//   R operator()(std::function<R(Args...)> f, Args... args) {
//     while (true) {
//     }
//   }
// };

// // TODO: add arguments and return value?
// template <size_t num_threads, typename R, typename... Args> class thread_pool {
//   std::array<std::thread, num_threads> threads;
//   std::array<bool, num_threads> is_busy;

//   add_work() {}
// };
}
