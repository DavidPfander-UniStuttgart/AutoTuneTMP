#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>
using namespace std; // to shorten code
using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;

#include "autotune/autotune.hpp"
using namespace autotune;
using namespace cppjit::builder;
#include "autotune/parameter.hpp"
#include "autotune/tuners/bruteforce.hpp"

// defines kernel, put in single compilation unit
AUTOTUNE_KERNEL(vector<double>(const size_t, const vector<double> &,
                               const vector<double> &),
                matrix_vector, "examples/kernel_matrix_vector")

vector<double> matrix_vector_reference(const vector<double> &m,
                                       const vector<double> &v) {

  const size_t N = v.size();
  vector<double> result(v.size());
#pragma omp parallel for
  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < N; j++) {
      result[i] += m[i * N + j] * v[j];
    }
  }
  return result;
}

int main(void) {

  matrix_vector.set_verbose(true);

  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution(0.0, 100.0);

  size_t repetitions = 100;
  size_t N = 8192; // L1 32kB -> max. 4096 double variables
                   // vector<double> m(N * N, 2.0);
                   // vector<double> v(N, 4.0);

  vector<double> m(N * N);
  std::fill(m.begin(), m.end(), distribution(generator));
  vector<double> v(N);
  std::fill(v.begin(), v.end(), distribution(generator));

  std::cout << "memory used \"m\": " << ((N * N * 8) / 1024) << "kB"
            << std::endl;
  std::cout << "memory used \"v\": " << ((N * 8) / 1024) << "kB" << std::endl;
  std::cout << "memory used (m, v, result): "
            << ((N * N * 8 + 2 * N * 8) / 1024) << "kB" << std::endl;
  vector<double> result;

  // matrix_vector.get_builder<gcc>().set_verbose(true);
  matrix_vector.get_builder<gcc>().set_cpp_flags(
      "-Wall -Wextra -fopenmp -std=c++17 -O3 -g "
      "-march=native -mtune=native -fstrict-aliasing ");
  matrix_vector.get_builder<gcc>().set_link_flags(
      "-std=c++17 -O3 -g -fopenmp -fstrict-aliasing ");
  matrix_vector.get_builder<gcc>().set_include_paths("-IVc_install/include");

  autotune::countable_set parameters;
  autotune::countable_continuous_parameter p1("BLOCKING", 4.0, 2.0, 1.0, 16.0,
                                              true);
  parameters.add_parameter(p1);
  autotune::tuners::bruteforce tuner(autotune::matrix_vector, parameters);
  tuner.set_repetitions(repetitions);
  tuner.set_verbose(true);
  autotune::countable_set optimal_parameters = tuner.tune(N, m, v);
  autotune::matrix_vector.set_parameter_values(optimal_parameters);
  std::cout << "optimal_parameters:" << std::endl;
  optimal_parameters.print_values();
  // autotune::matrix_vector.set_parameter_values(parameters);
  matrix_vector.compile();
  time_point start = high_resolution_clock::now();
  for (size_t i = 0; i < repetitions; i++) {
    result = matrix_vector(N, m, v);
  }
  time_point end = high_resolution_clock::now();
  double duration_s = std::chrono::duration<double>(end - start).count();
  std::cout << "avr. matrix_vector duration (native specified): "
            << (duration_s / repetitions) << std::endl;
  std::cout << "matrix_vector duration wallclock (native specified): "
            << duration_s << std::endl;

  std::vector<double> result_reference = matrix_vector_reference(m, v);

  bool all_ok = true;
  for (size_t i = 0; i < N; i++) {
    if (result[i] != result_reference[i]) {
      std::cout << "result[" << i << "] != " << result_reference[i]
                << std::endl;
      all_ok = false;
    }
  }
  if (all_ok) {
    std::cout << "comparison successful!" << std::endl;
  } else {
    std::cout << "comparison failed!" << std::endl;
  }
}
