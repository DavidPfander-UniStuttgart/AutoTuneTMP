#pragma once

#include <x86intrin.h>

namespace opttmp {
namespace bits {

inline uint64_t nthset_bit(uint64_t x, unsigned n) {
  return _pdep_u64(1ULL << n, x);
}

inline void set_bit(uint64_t &x, unsigned n) { x |= 1ULL << n; }

inline uint64_t ffs_0(uint64_t x) { return __builtin_ffs(x) - 1; }

void print_bits(uint64_t x) {
  for (size_t i = 0; i < 64; i += 1) {
    if (i % 8 == 0) {
      std::cout << "|";
    }
    uint64_t mask = 1ULL << i;
    // std::cout << std::endl << "mask:" << mask << std::endl;
    if ((x & mask) != 0) {
      std::cout << "1";
    } else {
      std::cout << "0";
    }
  }
  std::cout << "|";
}
} // namespace bits
} // namespace opttmp
