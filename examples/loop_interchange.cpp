#include <algorithm>
#include <array>
#include <iostream>
#include <tuple>
#include <vector>

#include "opttmp/loop/loop_exchange.hpp"

constexpr size_t N = 3;

namespace detail {
template <size_t depth> constexpr size_t to_perm_index_rec() { return 0; }

template <size_t depth, size_t index, size_t... indices>
constexpr size_t to_perm_index_rec() {
  return to_perm_index_rec<depth, indices...>() * depth + index;
  // return 1;
}
} // namespace detail

template <size_t depth, size_t... indices> constexpr size_t to_perm_index() {
  return detail::to_perm_index_rec<depth, indices...>();
  // return 5;
}

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

// namespace detail {
// template <class F, size_t depth, size_t cur_depth> // size_t order,
// void loop_interchange_rec(
//     F f, std::array<std::tuple<size_t, size_t, size_t>, depth> loop_bounds,
//     std::array<size_t, depth> order, std::array<size_t, depth> &index) {
//   if constexpr (cur_depth == depth) {
//     f(index);
//   } else {
//     for (size_t i = std::get<0>(loop_bounds[order[cur_depth]]);
//          i < std::get<1>(loop_bounds[order[cur_depth]]);
//          i += std::get<2>(loop_bounds[order[cur_depth]])) {
//       index[order[cur_depth]] = i;
//       loop_interchange_rec<F, depth, cur_depth + 1>(f, loop_bounds, order,
//                                                     index);
//     }
//   }
// }
// } // namespace detail

// // loop_bounds is min_1, max_1, min_2, max_2, ...
// template <class F, size_t depth> // size_t order,
// void loop_interchange(
//     F f, std::array<std::tuple<size_t, size_t, size_t>, depth> loop_bounds,
//     std::array<size_t, depth> order) {
//   std::array<size_t, depth> index;
//   detail::loop_interchange_rec<F, depth, 0>(f, loop_bounds, order, index);
// }

int main(void) {

  // for (size_t i = 0; i < N; i += 1) {
  //   for (size_t j = 0; j < N; j += 1) {
  //     for (size_t k = 0; k < N; k += 1) {
  //       std::cout << "i: " << i << " j: " << j << " k: " << k
  //                 << " id: " << (((i)*N + j) * N + k) << std::endl;
  //     }
  //   }
  // }
  // size_t encoded = to_perm_index<N, 2, 0, 1>();

  // std::cout << "N: " << N << " perm_N: " << encoded << std::endl;

  // size_t extracted = extract_index<N, 0>(encoded);
  // std::cout << "i = " << 0 << " extracted: " << extracted << std::endl;
  // extracted = extract_index<N, 1>(encoded);
  // std::cout << "i = " << 1 << " extracted: " << extracted << std::endl;
  // extracted = extract_index<N, 2>(encoded);
  // std::cout << "i = " << 2 << " extracted: " << extracted << std::endl;

  std::array<std::tuple<size_t, size_t, size_t>, N> loop_bounds{
      {{0, 2, 1}, {0, 2, 1}, {0, 2, 1}}};

  // 0, 1, 2
  // 0, 2, 1
  // 1, ...
  std::array<size_t, N> order{2, 0, 1};

  opttmp::loop::loop_interchange(
      [](std::array<size_t, N> &i) { // int x, int y, int z
        std::cout << "i[0]: " << i[0] << " i[1]: " << i[1] << " i[2]: " << i[2]
                  << std::endl;
      },
      loop_bounds, order);

  return 0;
}
