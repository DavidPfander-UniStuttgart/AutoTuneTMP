#include <algorithm>
#include <functional>
#include <vector>

#include "parameters.hpp"

#include <Vc/Vc>
using Vc::double_v;

using namespace std;

struct loop_spec {
  size_t start;
  size_t stop;
  size_t step;

  loop_spec(size_t start, size_t stop, size_t step)
      : start(start), stop(stop), step(step) {}

  loop_spec(size_t start, size_t stop) : start(start), stop(stop), step(1) {}
};

template <size_t blocking, typename F>
void loop_exchange(const loop_spec &outer, const loop_spec &inner, F body) {
  for (size_t i = outer.start; i < outer.stop; i += blocking)
    for (size_t j = inner.start; j < inner.stop; j += inner.step)
      for (size_t ii = 0; ii < blocking; ii += outer.step)
        body(i + ii, j);
}

extern "C" vector<double> matrix_vector(const vector<double> &m,
                                        const vector<double> &v) {
  const size_t N = v.size();
  vector<double> result(N, 0.0);

  // alternative: parallelize to saturate DRAM bandwidth w/o ILP
  // #pragma omp parallel for
  for (size_t i = 0; i < N; i += BLOCKING)
    for (size_t j = 0; j < N; j++)
      for (size_t k = 0; k < BLOCKING; k++)
        result[i + k] += m[(i + k) * N + j] * v[j];

  // // fastest version
  // for (size_t i = 0; i < N; i += BLOCKING) {
  //   array<double, BLOCKING> acc{};
  //   fill(result.begin() + i, result.begin() + i + BLOCKING, 0.0);
  //   for (size_t j = 0; j < N; j++)
  //     for (size_t k = 0; k < BLOCKING; k++)
  //       acc[k] += m[(i + k) * N + j] * v[j];
  //   // TODO: bug? slow for blocking == 1, ok blocking > 1
  //   copy(acc.begin(), acc.end(), result.begin() + i);
  //   // for (size_t k = 0; k < BLOCKING; k++)
  //   //   result[i + k] = acc[k];
  // }

  // for (size_t i = 0; i < N; i++)
  //   for (size_t j = 0; j < N; j++)
  //     result[i] += m[i * N + j] * v[j];

  loop_exchange<BLOCKING>(
      {0, N}, {0, N},
      [&](size_t i, size_t j) { result[i] += m[i * N + j] * v[j]; });

  return result;
}
