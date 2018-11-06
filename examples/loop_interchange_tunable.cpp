#include <algorithm>
#include <array>
#include <iostream>
#include <tuple>
#include <vector>

#include "autotune/autotune.hpp"
#include "autotune/continuous_parameter.hpp"
#include "autotune/tuners/bruteforce.hpp"

AUTOTUNE_KERNEL(void(), loop_interchange, "examples/loop_interchange_kernel")

constexpr size_t N = 3;

int to_perm_index(const std::vector<int> &perm) {
  int perm_index = perm[0];
  const int N = perm.size();
  for (size_t i = 1; i < static_cast<size_t>(N); i += 1) {
    perm_index = perm_index * N + perm[i];
  }
  return perm_index;
}

// namespace detail {
// template <size_t depth> constexpr size_t to_perm_index_rec() { return 0; }

// template <size_t depth, size_t index, size_t... indices>
// constexpr size_t to_perm_index_rec() {
//   return to_perm_index_rec<depth, indices...>() * depth + index;
//   // return 1;
// }
// } // namespace detail

// template <size_t depth, size_t... indices> constexpr size_t to_perm_index() {
//   return detail::to_perm_index_rec<depth, indices...>();
//   // return 5;
// }

namespace detail {

template <size_t N>
void get_permutations_rec(std::vector<std::vector<int>> &result,
                          std::vector<int> &v, std::vector<int> &p,
                          std::vector<bool> &taken) {
  if (p.size() == N) {
    result.push_back(p);
  } else {
    for (size_t i = 0; i < N; i += 1) {
      if (!taken[i]) {
        // take ith element
        taken[i] = true;
        p.push_back(v[i]);
        get_permutations_rec<N>(result, v, p, taken);
        p.pop_back();
        taken[i] = false;
      }
    }
  }
}
} // namespace detail

template <size_t N>
std::vector<std::vector<int>> get_permutations(std::vector<int> v) {
  std::vector<std::vector<int>> result;
  std::vector<int> p;
  std::vector<bool> taken(N, false);
  detail::get_permutations_rec<N>(result, v, p, taken);
  return result;
}

int main(void) {

  autotune::loop_interchange.set_verbose(true);
  auto &builder =
      autotune::loop_interchange.get_builder<cppjit::builder::gcc>();
  // assuming the example is run from the repository base folder
  builder.set_include_paths("-I include");
  builder.set_cpp_flags("-std=c++17 -fPIC");
  builder.set_link_flags("-shared");

  std::vector<int> order{0, 1, 2};
  auto permutations = get_permutations<3>(order);
  std::cout << "all permutations:" << std::endl;
  for (size_t i = 0; i < permutations.size(); i += 1) {
    std::vector<int> &p = permutations[i];
    for (size_t j = 0; j < p.size(); j += 1) {
      if (j > 0) {
        std::cout << ", ";
      }
      std::cout << p[j];
    }
    std::cout << std::endl;
  }
  std::vector<int> perm_indices;
  for (size_t i = 0; i < permutations.size(); i += 1) {
    int perm_index = to_perm_index(permutations[i]);
    perm_indices.push_back(perm_index);
  }
  std::cout << "all encoded permutations:" << std::endl;
  for (size_t i = 0; i < perm_indices.size(); i += 1) {
    if (i > 0) {
      std::cout << ", ";
    }
    std::cout << perm_indices[i];
  }
  std::cout << std::endl;

  autotune::countable_set parameters;
  autotune::fixed_set_parameter<int> p1("LOOP_ORDER", perm_indices);
  parameters.add_parameter(p1);

  autotune::tuners::bruteforce tuner(autotune::loop_interchange, parameters);
  tuner.set_verbose(true);
  tuner.tune();

  // autotune::loop_interchange();

  return 0;
}
