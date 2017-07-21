#include "opttmp/loop/unroll_loop.hpp"
#include "autotune/autotune.hpp"
#include "autotune/parameter.hpp"

#include "cppjit/cppjit.hpp"

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

  auto builder = std::static_pointer_cast<cppjit::builder::gcc>(
      autotune::unrolling_kernel.get_builder());
  builder->set_verbose(true);
  // assuming the example is run from the repository base folder
  builder->set_include_paths("-I src");

  autotune::unrolling_kernel.add_parameter("UNROLL_LOOP", {"0", "1"});

  autotune::unrolling_kernel.add_parameter("DUMMY_PAR_2", {"0", "1", "2", "3"});

  autotune::unrolling_kernel.add_parameter("DUMMY_PAR_3", {"a", "b", "c", "d"});

  // tune kernel, note that arguments are reused
  autotune::unrolling_kernel.tune(arr, N);

  autotune::unrolling_kernel.compile("examples/kernels_unroll_loop_autotune/");

  autotune::unrolling_kernel.print_parameters();
  autotune::unrolling_kernel(arr, N);
  return 0;
}
