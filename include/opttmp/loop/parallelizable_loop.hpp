#include "autotune/thread_pool.hpp"

namespace opttmp {
namespace loop {

template <bool parallelize, class F, size_t num_threads>
void parallizable_loop(autotune::simple_thread_pool<num_threads> &pool,
                       std::tuple<size_t, size_t, size_t> loop_bounds, F f) {
  if (parallelize) {
    for (size_t i = std::get<0>(loop_bounds); i < std::get<1>(loop_bounds);
         i += std::get<2>(loop_bounds)) {
      pool.enqueue_work(f, i);
    }
  } else {
    for (size_t i = std::get<0>(loop_bounds); i < std::get<1>(loop_bounds);
         i += std::get<2>(loop_bounds)) {
      f(i);
    }
  }
}
} // namespace loop
} // namespace opttmp
