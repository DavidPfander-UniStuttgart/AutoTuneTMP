#pragma once

#include "autotune/thread_meta.hpp"

// only to be included from kernel ".cpp" file

namespace detail {
autotune::thread_meta meta;
}

extern "C" void set_meta(autotune::thread_meta meta) { detail::meta = meta; }

extern "C" autotune::thread_meta get_meta() { return detail::meta; }
