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
  std::vector<double> m(N * N);
  std::fill(m.begin(), m.end(), distribution(generator));

  tiling_configuration conf = {{2, N}, {4, N}};
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

  std::vector<double> back = memory_layout::undo_tiling<2>(tiled, conf);

  std::cout << "colored matrix:" << std::endl;
  for (size_t i = 0; i < N; i++) {
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
