#pragma once

#include <cstddef>
#include <iostream>
#include <utility>

#include "../helper.hpp"

namespace opttmp {
namespace loop {

namespace detail {
template <class F, size_t from, size_t increment, std::size_t... I>
void unroll_loop(F f, std::index_sequence<I...>) {
  // nicer C++17 version
  // (f(from + I * increment), ...);

  // C++ TMP doesn't immediately permit unpacking in multiple statements
  // but it does allow unpacking in argument list for funtion
  // swallow consumes the arguments as separated list (we want the side-effects)
  // ", 0" is required because a void-expression cannot be a function argument
  opttmp::detail::swallow((f(from + I * increment), 0)...);
}

template <class F, size_t from, size_t increment, std::size_t... I>
void unroll_loop_template(F &f, std::index_sequence<I...>) {
  // nicer C++17 version
  // (f(from + I * increment), ...);

  // C++ TMP doesn't immediately permit unpacking in multiple statements
  // but it does allow unpacking in argument list for funtion
  // swallow consumes the arguments as separated list (we want the side-effects)
  // ", 0" is required because a void-expression cannot be a function argument
  // constexpr size_t cur_index = from + I * increment;

  // opttmp::detail::swallow((f(from + I * increment), 0)...);

  opttmp::detail::swallow(
      (f.template operator() < from + I * increment > (), 0)...);
}
}

template <size_t from, size_t to, size_t increment, class F>
void unroll_loop(F f) {
  detail::unroll_loop<F, from, increment>(
      f, std::make_index_sequence<(to - from) / increment>());
}

template <size_t from, size_t to, size_t increment, class F>
void unroll_loop_template(F &f) {
  detail::unroll_loop_template<F, from, increment>(
      f, std::make_index_sequence<(to - from) / increment>());
}
}
}
