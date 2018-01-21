#include <algorithm>
#include <functional>
#include <vector>

#include <Vc/Vc>
using Vc::double_v;

using namespace std;

constexpr size_t blocking = 1;

extern "C" vector<double> matrix_vector(const vector<double> &m,
                                        const vector<double> &v) {
  const size_t N = v.size();
  vector<double> result(v.size());
// #pragma omp parallel for
  for (size_t i = 0; i < N; i += blocking) {
    std::array<double, blocking> acc{};
    for (size_t j = 0; j < N; j++) {
      for (size_t k = 0; k < blocking; k++) {
        acc[k] += m[(i + k) * N + j] * v[j];
      }
    }
    for (size_t k = 0; k < blocking; k++) {
      result[i + k] = acc[k];
    }
  }
  return result;
}
