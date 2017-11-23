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
  (f(from + I * increment), ...);
}
}

template <size_t from, size_t to, size_t increment, class F>
void unroll_loop(F f) {
  detail::unroll_loop<F, from, increment>(
      f, std::make_index_sequence<(to - from) / increment>());
}

namespace detail {
template <class F, size_t from, size_t increment, std::size_t... I>
void unroll_loop_template(const F &f, std::index_sequence<I...>) {
  (f.template operator() < from + I * increment > (), ...);
}
}

template <size_t from, size_t to, size_t increment, class F>
void unroll_loop_template(const F &f) {
  detail::unroll_loop_template<F, from, increment>(
      f, std::make_index_sequence<(to - from) / increment>());
}
}
}

// needed, because templates cannot be passed as arguments
#define DEFINE_LOOP_BODY(function_name)                                        \
  template <typename... Ts> struct function_name##_body {                      \
    std::tuple<Ts &...> vars;                                                  \
    function_name##_body() = delete;                                           \
    function_name##_body(function_name##_body<Ts...> &other) = delete;         \
    function_name##_body(Ts &... args) : vars(args...) {}                      \
    template <size_t i> void operator()() const {                              \
      opttmp::detail::call_tuple(function_name<i>, vars,                       \
                                 std::make_index_sequence<sizeof...(Ts)>());   \
    }                                                                          \
  };
