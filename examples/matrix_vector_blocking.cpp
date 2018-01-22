#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>
using namespace std; // to shorten code
using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;

#include "autotune/autotune.hpp"
using namespace autotune;
using namespace cppjit::builder;
#include "autotune/parameter.hpp"
#include "autotune/tuners/line_search.hpp"

// defines kernel, put in single compilation unit
AUTOTUNE_DECLARE_DEFINE_KERNEL_SRC(vector<double>(size_t, vector<double> &,
                                                  vector<double> &),
                                   matrix_vector,
                                   "examples/kernel_matrix_vector")

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

  size_t repetitions = 100;
  const size_t N = 8192; // L1 32kB -> max. 4096 double variables
  vector<double> m(N * N, 2.0);
  vector<double> v(N, 4.0);

  std::cout << "memory used \"m\": " << ((N * N * 8) / 1024) << "kB"
            << std::endl;
  std::cout << "memory used \"v\": " << ((N * 8) / 1024) << "kB" << std::endl;
  std::cout << "memory used (m, v, result): "
            << ((N * N * 8 + 2 * N * 8) / 1024) << "kB" << std::endl;
  vector<double> result;

  matrix_vector.get_builder<gcc>().set_verbose(true);
  matrix_vector.get_builder<gcc>().set_cpp_flags(
      "-Wall -Wextra -fopenmp -std=c++17 -O3 -g "
      "-march=native -mtune=native -fstrict-aliasing ");
  matrix_vector.get_builder<gcc>().set_link_flags(
      "-std=c++17 -O3 -g -fopenmp -fstrict-aliasing ");
  matrix_vector.get_builder<gcc>().set_include_paths("-IVc_install/include");
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
