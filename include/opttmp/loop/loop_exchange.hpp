#pragma once

namespace opttmp {
namespace loop {
namespace detail {
template <class F, size_t depth, size_t cur_depth> // size_t order,
void loop_interchange_rec(
    F f, std::array<std::tuple<size_t, size_t, size_t>, depth> loop_bounds,
    std::array<size_t, depth> order, std::array<size_t, depth> &index) {
  if constexpr (cur_depth == depth) {
    f(index);
  } else {
    for (size_t i = std::get<0>(loop_bounds[order[cur_depth]]);
         i < std::get<1>(loop_bounds[order[cur_depth]]);
         i += std::get<2>(loop_bounds[order[cur_depth]])) {
      index[order[cur_depth]] = i;
      loop_interchange_rec<F, depth, cur_depth + 1>(f, loop_bounds, order,
                                                    index);
    }
  }
}
} // namespace detail

// loop_bounds is min_1, max_1, min_2, max_2, ...
template <class F, size_t depth> // size_t order,
void loop_interchange(
    F f, std::array<std::tuple<size_t, size_t, size_t>, depth> loop_bounds,
    std::array<size_t, depth> order) {
  std::array<size_t, depth> index;
  detail::loop_interchange_rec<F, depth, 0>(f, loop_bounds, order, index);
}

} // namespace loop
} // namespace opttmp
