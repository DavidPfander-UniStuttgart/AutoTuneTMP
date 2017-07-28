/*
 * new.cpp
 *
 *  Created on: Oct 1, 2015
 *      Author: dmarce1
 */

#include <Vc/Vc>
#include <cstdio>
#include <cstdlib>

#if defined(Vc_HAVE_AVX512F)
constexpr std::size_t alignment = 64;
#else
constexpr std::size_t alignment = 32;
#endif

static void *allocate(std::size_t);
static void deallocate(void *);

void *operator new(std::size_t n) { return allocate(n); }

void *operator new[](std::size_t n) { return allocate(n); }

void operator delete(void *p) { deallocate(p); }

void operator delete(void *p, std::size_t) { deallocate(p); }

void operator delete[](void *p) { deallocate(p); }

void operator delete[](void *p, std::size_t) { deallocate(p); }

static void *allocate(std::size_t n) {
  // std::cout << "in allocate" << std::endl;
  void *ptr;
  if (posix_memalign(&ptr, alignment, n) != 0) {
    printf("posix_memalign failed!\n");
    abort();
  }
  return ptr;
}

static void deallocate(void *ptr) { free(ptr); }
