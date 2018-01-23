#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>
using namespace std; // to shorten code

#include "autotune/autotune.hpp"
using namespace autotune;
using namespace cppjit::builder;
#include "autotune/parameter.hpp"
#include "autotune/tuners/bruteforce.hpp"

// defines kernel, put in single compilation unit
AUTOTUNE_KERNEL(vector<double>(const vector<double> &, const vector<double> &),
                matrix_vector, "examples/kernel_simplified_matrix_vector")

int main(void) {
  size_t N = 8192;
  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution(0.0, 100.0);
  vector<double> m(N * N);
  std::fill(m.begin(), m.end(), distribution(generator));
  vector<double> v(N);
  std::fill(v.begin(), v.end(), distribution(generator));

  matrix_vector.get_builder<gcc>().set_cpp_flags(
      "-std=c++17 -O3 -march=native -mtune=native ");
  matrix_vector.get_builder<gcc>().set_link_flags("-std=c++17 -O3 ");
  matrix_vector.get_builder<gcc>().set_include_paths("-IVc_install/include");

  autotune::countable_set parameters;
  parameters.emplace_parameter<autotune::countable_continuous_parameter>(
      "BLOCKING", 1.0, 2.0, 1.0, 16.0, true);
  tuners::bruteforce tuner(autotune::matrix_vector, parameters);
  tuner.set_repetitions(100); // for more accurate tuning
  countable_set optimal_parameters = tuner.tune(m, v);
  matrix_vector.set_parameter_values(optimal_parameters);
  vector<double> result = matrix_vector(m, v);
}
