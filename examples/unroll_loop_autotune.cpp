#include "autotune/autotune.hpp"
#include "autotune/parameter.hpp"
#include "autotune/tuners/line_search.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

// defines kernel, put in single compilation unit
AUTOTUNE_DECLARE_DEFINE_KERNEL(void(std::vector<double> &, const size_t),
                               unrolling_kernel)

int main(void) {
  size_t N = 20;

  std::vector<double> arr(N);
  std::fill(arr.begin(), arr.end(), 0.0);

  autotune::unrolling_kernel.set_verbose(true);

  auto builder =
      autotune::unrolling_kernel.get_builder_as<cppjit::builder::gcc>();
  // assuming the example is run from the repository base folder
  builder->set_include_paths("-I src");
  builder->set_cpp_flags("-std=c++17 -fPIC");
  builder->set_link_flags("-shared");

  autotune::unrolling_kernel.set_source_dir(
      "examples/kernels_unroll_loop_autotune");

  autotune::unrolling_kernel.add_parameter("UNROLL_LOOP", {"0", "1"});
  autotune::unrolling_kernel.add_parameter("DUMMY_PAR_2", {"0", "1", "2", "3"});
  autotune::unrolling_kernel.add_parameter("DUMMY_PAR_3", {"a", "b", "c", "d"});

  std::vector<size_t> line_search_initial_guess = {0, 0, 0};
  autotune::tuners::line_search<decltype(autotune::unrolling_kernel)> tuner(
      autotune::unrolling_kernel, 50, 1, line_search_initial_guess);
  std::vector<size_t> optimal_indices = tuner.tune(arr, N);

  autotune::unrolling_kernel.print_values(optimal_indices);
  return 0;
}
