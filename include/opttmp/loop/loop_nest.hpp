#pragma once

#include <array>
#include <cstddef>
#include <utility>

namespace opttmp {
namespace loop {

namespace detail {

// template recursion for std::array
template <size_t dim, size_t cur_dim, typename index_type, typename F>
typename std::enable_if<cur_dim == dim, void>::type
dynamic_loop_nest(const std::array<index_type, dim> &,
                  const std::array<index_type, dim> &,
                  const std::array<index_type, dim> &,
                  std::array<index_type, dim> &completed_index, F f) {
  f(completed_index);
}

template <size_t dim, size_t cur_dim, typename index_type, typename F>
typename std::enable_if<cur_dim != dim, void>::type
dynamic_loop_nest(const std::array<index_type, dim> &min,
                  const std::array<index_type, dim> &max,
                  const std::array<index_type, dim> &step,
                  std::array<index_type, dim> &partial_index, F f) {
  for (index_type cur = min[cur_dim]; cur < max[cur_dim];
       cur += step[cur_dim]) {
    partial_index[cur_dim] = cur;
    dynamic_loop_nest<dim, cur_dim + 1>(min, max, step, partial_index, f);
  }
}
}

// overload for std::array
template <size_t dim, typename index_type, typename F>
void dynamic_loop_nest(const std::array<index_type, dim> &min,
                       const std::array<index_type, dim> &max,
                       const std::array<index_type, dim> &step, F f) {
  std::array<index_type, dim> partial_index;
  detail::dynamic_loop_nest<dim, 0>(min, max, step, partial_index, f);
}

} // namespace loop
} // namespace opttmp
