#include <iostream>

#include <opttmp/loop/loop_tiling.hpp>

using namespace opttmp::loop;

int main(void) {
  tiled_iterator l0(0, 8, 4);
  tiled_iterator l1(0, 4, 2);
  // tiled_iterator l3(0, 12, 4);
  tiled_iterator &l0_p = l0.tile(2);
  tiled_iterator &l0_pp = l0_p.tile(1);
  tiled_iterator &l1_p = l1.tile(1);
  // while (l0.has_next()) {
  //   size_t value = l0.next();
  //   while (l0_p.has_next()) {
  //     size_t value_p = l0_p.next();
  //     while (l0_pp.has_next()) {
  //       size_t value_pp = l0_pp.next();
  //       std::cout << "value_pp: " << value_pp << ", value_p:" << value_p
  //                 << ", value: " << value << std::endl;
  //     }
  //   }
  // }

  loop_index li(l0, l1, l0_p, l0_pp, l1_p);
  li.iterate([](std::array<size_t, 5> &indices) {
    std::cout << "i0: " << indices[0] << ", i1:" << indices[1]
              << ", i2: " << indices[2] << ", i3: " << indices[3]
              << ", i4: " << indices[4] << std::endl;
  });

  return EXIT_SUCCESS;
}
