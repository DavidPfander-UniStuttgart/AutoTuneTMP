#pragma once

#include <x86intrin.h>

/*
usage:
- measure clock base rate with the rdtscp_* calls
- start and stop version serialize in different order (cpuid serializes),
subtract for base clock cycles
- measure the actual clockrate via rdpmc (two calls, subtract for actual cycles)
- rdpmc only works on Intel and it requires rdpcm to be available in usermode
(otherwise will result in a segmentation fault)
(- might additionally need chrono::high_resolution_clock::now() for pure timing)
 */

static inline unsigned long long rdtscp_base_cycles_start() {
  unsigned cycles_high;
  unsigned cycles_low;
  asm volatile("CPUID\n\t"
               "RDTSC\n\t"
               "mov %%edx, %0\n\t"
               "mov %%eax, %1\n\t"
               : "=r"(cycles_high), "=r"(cycles_low)::"%rax", "%rbx", "%rcx",
                 "%rdx");

  uint64_t start = ((static_cast<uint64_t>(cycles_high) << 32) | cycles_low);
  return start;
}

static inline unsigned long long rdtscp_base_cycles_stop() {
  uint32_t cycles_high;
  uint32_t cycles_low;
  asm volatile("RDTSCP\n\t"
               "mov %%edx,  %0\n\t "
               "mov %%eax,  %1\n\t "
               "CPUID\n\t"
               : "=r"(cycles_high), "=r"(cycles_low)::"%rax", "%rbx", "%rcx",
                 "%rdx");
  uint64_t stop = (static_cast<uint64_t>(cycles_high) << 32) | cycles_low;
  return stop;
}

#ifdef USERMODE_RDPMC_ENABLED
uint64_t inline rdpmc_actual_cycles() {
  uint32_t a, d, c;
  c = (1 << 30) + 1;
  __asm__ volatile("rdpmc" : "=a"(a), "=d"(d) : "c"(c));
  return static_cast<uint64_t>(a) | (static_cast<uint64_t>(d) << 32);
}
#endif
