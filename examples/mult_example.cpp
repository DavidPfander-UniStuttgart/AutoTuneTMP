#include <algorithm>
#include <iostream>
#include <vector>

#include "autotune/autotune.hpp"
#include "autotune/parameter.hpp"
#include "autotune/tuners/bruteforce.hpp"
#include "autotune/tuners/countable_set.hpp"

AUTOTUNE_KERNEL(void(size_t, std::vector<double> &, size_t,
                     std::vector<double> &, std::vector<double> &,
                     std::vector<double> &, std::vector<double> &),
                mult_kernel, "examples/mult_kernel")

int main(void) {
  std::cout << "testing countable set interface" << std::endl;
  autotune::mult_kernel.set_verbose(true);
  auto &builder = autotune::mult_kernel.get_builder<cppjit::builder::gcc>();
  // builder.set_verbose(true);
  builder.set_include_paths(
      "-I/home/pfandedd/git/AutoTuneTMP/AutoTuneTMP_install_debug/include "
      "-I/home/pfandedd/git/AutoTuneTMP/Vc_install/include "
      "-I/home/pfandedd/git/AutoTuneTMP/boost_install/include");
  builder.set_cpp_flags(
      "-Wall -Wextra -std=c++17 -march=native -mtune=native "
      "-O3 -g -ffast-math  -fPIC -fno-gnu-unique");   // -fopenmp
  builder.set_link_flags("-shared -fno-gnu-unique "); // -fopenmp

  autotune::countable_set parameters;
  autotune::fixed_set_parameter<size_t> p1("DATA_BLOCKING", {6});
  parameters.add_parameter(p1);

  // std::vector<size_t> thread_values{3, 100};
  autotune::fixed_set_parameter<size_t> p2("KERNEL_OMP_THREADS", {3, 100});
  parameters.add_parameter(p2);

  autotune::mult_kernel.set_parameter_values(parameters);

  size_t dims = 2;
  size_t dataset_size = 16;
  size_t grid_size = 16;
  std::vector<double> dataset_SoA(dataset_size);

  std::vector<double> level_list(grid_size);
  std::vector<double> index_list(grid_size);
  std::vector<double> alpha(grid_size);
  std::vector<double> result_padded(dataset_size);

  dataset_size = 0;

  autotune::mult_kernel(dims, dataset_SoA, dataset_size, level_list, index_list,
                        alpha, result_padded);

  parameters[1]->next();

  // compile beforehand so that compilation is not part of the measured duration
  autotune::mult_kernel.set_parameter_values(parameters);
  autotune::mult_kernel(dims, dataset_SoA, dataset_size, level_list, index_list,
                        alpha, result_padded);

  return 0;
}
