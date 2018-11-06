#include "parameters.hpp"

#include <array>
#include <iostream>
#include <tuple>

#include "opttmp/loop/loop_exchange.hpp"

namespace detail {
template <size_t depth, size_t index, size_t cur_depth>
constexpr size_t extract_index_rec(const size_t perm_rem) {
  if constexpr (cur_depth == index) {
    return perm_rem % depth;
  } else {
    return extract_index_rec<depth, index, cur_depth + 1>(perm_rem / depth);
  }
}
} // namespace detail

template <size_t depth, size_t index>
constexpr size_t extract_index(const size_t perm_index) {
  return detail::extract_index_rec<depth, index, 0>(perm_index);
}

constexpr size_t N = 3;

AUTOTUNE_EXPORT void loop_interchange() {
  std::array<std::tuple<size_t, size_t, size_t>, N> loop_bounds{
      {{0, 2, 1}, {0, 2, 1}, {0, 2, 1}}};

  // 0, 1, 2
  // 0, 2, 1
  // 1, ...
  // std::array<size_t, N> order{2, 0, 1};
  std::array<size_t, N> order;
  order[0] = extract_index<N, 0>(LOOP_ORDER);
  order[1] = extract_index<N, 1>(LOOP_ORDER);
  order[2] = extract_index<N, 2>(LOOP_ORDER);

  opttmp::loop::loop_interchange(
      [](std::array<size_t, N> &i) { // int x, int y, int z
        std::cout << "i[0]: " << i[0] << " i[1]: " << i[1] << " i[2]: " << i[2]
                  << std::endl;
      },
      loop_bounds, order);
}
