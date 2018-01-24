#include "opttmp/memory_layout/tile_array.hpp"

#include <iostream>
#include <random>
#include <vector>

using namespace memory_layout;
using namespace std;

int main(void) {

  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution(0.0, 100.0);

  size_t N = 16;
  size_t M = 8;
  std::vector<double> m(M * N);
  std::fill(m.begin(), m.end(), distribution(generator));

  // {blocked_rows, total_rows}, {blocked_columns, total_columns}
  tiling_configuration conf = {{2, M}, {4, N}};
  std::vector<double> tiled = memory_layout::make_tiled<2>(m, conf);

  size_t color = 0;
  memory_layout::iterate_tiles<2>(tiled, conf, [&color](auto view) {
    for (size_t i = 0; i < 2; i++) {
      for (size_t j = 0; j < 4; j++) {
        view[i * 4 + j] = color;
      }
    }
    color += 1;
  });

  auto back = memory_layout::undo_tiling<2>(tiled, conf);

  std::cout << "colored matrix:" << std::endl;
  for (size_t i = 0; i < M; i++) {
    for (size_t j = 0; j < N; j++) {
      if (j > 0) {
        std::cout << " ";
      }
      if (back[i * N + j] < 10) {
        std::cout << " ";
      }
      if (back[i * N + j] < 100) {
        std::cout << " ";
      }
      std::cout << back[i * N + j];
    }
    std::cout << std::endl;
  }

  return 0;
}
