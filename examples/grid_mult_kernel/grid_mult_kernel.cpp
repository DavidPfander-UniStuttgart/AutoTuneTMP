#include "grid.hpp"
#include "autotune_kernel.hpp"

#include <atomic>
#include <vector>

#include <Vc/Vc>
using Vc::double_v;

template <typename T>
void add_atomically(std::vector<T> &m, size_t base_index,
                    double_v value_to_add) {
  for (size_t i = 0; i < double_v::size(); i++) {
    // vector width many atomic updates
    while (true) {
      double org = m[base_index + i];
      double new_val = org + value_to_add[i];
      if (m[base_index + i].compare_exchange_weak(org, new_val)) {
        break;
      }
    }
  }
}

AUTOTUNE_EXPORT void grid_mult_kernel(std::vector<double> &A,
                                      std::vector<double> &B,
                                      std::vector<std::atomic<double>> &C,
                                      size_t N, size_t z_block_size) {
  autotune::thread_meta meta = get_meta();
  // std::cout << "meta.z: " << meta.z << std::endl;
  // std::cout << "meta.y: " << meta.y << std::endl;
  // std::cout << "meta.x: " << meta.x << std::endl;

  // std::cout << "info: vector size IN KERNEL is: " << double_v::size()
  //           << std::endl;
  // std::cout << "DUMMY_PARAMETER: " << DUMMY_PARAMETER << std::endl;

  double_v C_comp(0.0);
  size_t z_base = meta.z * z_block_size;

  for (size_t i = z_base; i < z_base + z_block_size; i++) {
    double_v A_comp(A[meta.y * N + i]);
    double_v B_comp(&B[i * N + meta.x], Vc::flags::element_aligned);
    C_comp += A_comp * B_comp;
  }
  add_atomically(C, meta.y * N + meta.x, C_comp);
}
