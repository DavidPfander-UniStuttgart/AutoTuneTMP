#pragma once

#include "../parameter.hpp"
#include "common.hpp"

namespace autotune {

template <class F, class test, typename... Args>
std::vector<size_t> line_search(F f, test t, const Args &... args) {
  throw;
}
}
