#include "opttmp/loop/unroll_loop.hpp"
#include "autotune/autotune.hpp"
#include "autotune/parameter.hpp"

#include "cppjit/cppjit.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

// declares kernel, provide function type and a c-identifier as name
CPPJIT_DECLARE_KERNEL(void(std::vector<double> &, const size_t), unrolling_kernel)

AUTOTUNE_DEFINE_KERNEL(set_three)

AUTOTUNE_ADD_PARAMETER(set_three, UNROLL_LOOP,
                       std::vector<std::string>({"0", "1"}))

int main(void) {
  constexpr size_t N = 100;

  std::vector<double> arr(N);
  std::fill(arr.begin(), arr.end(), 0.0);

  auto builder = std::static_pointer_cast<cppjit::builder::gcc>(cppjit::get_builder_unrolling_kernel());
  builder->set_verbose(true);
  builder->set_include_paths("-I src");

  compile_inline_unrolling_kernel(
      "#include \"opttmp/loop/unroll_loop.hpp\"\n"
      "#include <vector>\n"
      "extern \"C\" void unrolling_kernel(std::vector<double> &arr, const size_t N) {\n"
      "  opttmp::loop::unroll_loop<10, 40, 2>([&arr](auto i) {\n"
      "    arr[i] = 3.0; });\n"
      "  for (size_t i = 0; i < N; i++) {\n"
      "    std::cout << \"i: \" << i << \" -> \" << arr[i] << std::endl;\n"
      "  }\n"
      "}\n");

  autotune::kernels::set_three.print_parameters();

  unrolling_kernel(arr, N);

  return 0;
}

// defines kernel, put in single compilation unit
CPPJIT_DEFINE_KERNEL(void(std::vector<double> &, const size_t), unrolling_kernel)
