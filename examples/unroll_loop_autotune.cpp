#include "autotune/autotune.hpp"
#include "autotune/parameter.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

// defines kernel, put in single compilation unit
AUTOTUNE_DECLARE_DEFINE_KERNEL(void(std::vector<double> &, const size_t),
                               unrolling_kernel)

int main(void) {
  constexpr size_t N = 20;

  std::vector<double> arr(N);
  std::fill(arr.begin(), arr.end(), 0.0);

  autotune::unrolling_kernel.set_verbose(true);

  // assuming the example is run from the repository base folder
  autotune::unrolling_kernel.get_builder_as<cppjit::builder::gcc>()
      ->set_include_paths("-I src");

  autotune::unrolling_kernel.set_source_dir(
      "examples/kernels_unroll_loop_autotune");

  autotune::unrolling_kernel.add_parameter("UNROLL_LOOP", {"0", "1"});
  autotune::unrolling_kernel.add_parameter("DUMMY_PAR_2", {"0", "1", "2", "3"});
  autotune::unrolling_kernel.add_parameter("DUMMY_PAR_3", {"a", "b", "c", "d"});

  // autotune::unrolling_kernel.compile("examples/kernels_unroll_loop_autotune/");

  // tune kernel, note that arguments are reused
  std::vector<size_t> optimal_indices =
      autotune::unrolling_kernel.tune(autotune::tuner::bruteforce, arr, N);

  autotune::unrolling_kernel.print_values(optimal_indices);
  // autotune::unrolling_kernel.print_parameters();

  // std::cout << "now compiling and running..." << std::endl;

  // std::vector<size_t> indices = {0, 0, 0};
  // autotune::unrolling_kernel.create_parameter_file(indices);
  // autotune::unrolling_kernel.compile();
  // autotune::unrolling_kernel(arr, N);
  return 0;
}
