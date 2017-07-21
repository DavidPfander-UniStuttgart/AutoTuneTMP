#include "autotune/autotune.hpp"
#include "autotune/parameter.hpp"
#include "opttmp/loop/unroll_loop.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

AUTOTUNE_DECLARE_DEFINE_KERNEL(void(std::vector<double> &, const size_t),
                               unrolling_kernel)

int main(void) {
  constexpr size_t N = 50;

  std::vector<double> arr(N);
  std::fill(arr.begin(), arr.end(), 0.0);

  auto builder =
      autotune::unrolling_kernel.get_builder_as<cppjit::builder::gcc>();
  builder->set_verbose(true);
  builder->set_include_paths("-I src");

  autotune::unrolling_kernel.add_parameter(
      "UNROLL_LOOP", std::vector<std::string>({"0", "1"}));

  autotune::unrolling_kernel.compile_inline(
      "#include \"opttmp/loop/unroll_loop.hpp\"\n"
      "#include <vector>\n"
      "extern \"C\" void unrolling_kernel(std::vector<double> &arr, const "
      "size_t N) {\n"
      "  opttmp::loop::unroll_loop<10, 40, 2>([&arr](auto i) {\n"
      "    arr[i] = 3.0; });\n"
      "  for (size_t i = 0; i < N; i++) {\n"
      "    std::cout << \"i: \" << i << \" -> \" << arr[i] << std::endl;\n"
      "  }\n"
      "}\n");

  autotune::unrolling_kernel.print_parameters();

  autotune::unrolling_kernel(arr, N);

  return 0;
}
