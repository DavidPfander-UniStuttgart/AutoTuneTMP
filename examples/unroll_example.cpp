#include "opttmp/loop/unroll_loop.hpp"

#include <algorithm>
#include <iostream>
#include <tuple>
#include <vector>

template <class F> void executor123(F f) {
  f.template operator()<0>();
  f.template operator()<1>();
  f.template operator()<2>();
  // template f<1>();
  // template f<2>();
}

template <typename... Ts> struct body {

  std::tuple<Ts &...> vars;

  body(Ts... args) : vars(args...) {}

  template <size_t i> void operator()() {
    std::cout << "i: " << i << " -> arr[" << i << "]: " << std::get<0>(vars)[i]
              << std::endl;
  }
};

int main(void) {
  constexpr size_t N = 100;

  std::vector<double> arr(N);
  std::fill(arr.begin(), arr.end(), 0.0);

  for (size_t i = 0; i < 3; i++) {
    arr[i] = 3.0;
  }

  opttmp::loop::unroll_loop<10, 40, 2>([&arr](auto i) { arr[i] = 3.0; });

  // for (size_t i = 0; i < N; i++) {
  //   std::cout << "i: " << i << " -> " << arr[i] << std::endl;
  // }

  // f<3>();
  body<std::vector<double>> b(arr);
  // executor123(b);
  opttmp::loop::unroll_loop_template<10, 40, 2>(b);

  return 0;
}
