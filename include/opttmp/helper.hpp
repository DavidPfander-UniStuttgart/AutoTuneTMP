#pragma once

#include <tuple>

namespace opttmp {
namespace detail {

template <typename... Ts> void swallow(const Ts &...) {}

template <typename F, typename... Ts, size_t... Indices>
void call_tuple(F f, std::tuple<Ts...> vars, std::index_sequence<Indices...>) {
  f(std::get<Indices>(vars)...);
}
}
}
