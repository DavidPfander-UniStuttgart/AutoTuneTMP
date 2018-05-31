#pragma once

#include <map>
#include <mutex>

#include "autotune/thread_meta.hpp"

// only to be included from kernel ".cpp" file

namespace detail {
// std::map<int64_t, autotune::thread_meta> metas;
// std::mutex access_map_mutex;
thread_local int64_t thread_id;
thread_local autotune::thread_meta meta;
} // namespace detail

extern "C" void set_meta(autotune::thread_meta meta) { // , int64_t thread_id
  // std::unique_lock lock(detail::access_map_mutex);
  // detail::metas[thread_id] = meta;
  detail::meta = meta;
}

autotune::thread_meta get_meta() { // int64_t thread_id
  // std::unique_lock lock(detail::access_map_mutex);
  // return detail::metas[thread_id];
  return detail::meta;
}

extern "C" size_t set_thread_id(int64_t thread_id) {
  detail::thread_id = thread_id;
}

size_t get_thread_id() { return detail::thread_id; }
