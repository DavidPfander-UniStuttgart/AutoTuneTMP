#include "parameters.hpp"

AUTOTUNE_EXPORT int add_one(const int a) {
#if ADD_ONE == 1
  return a + 1;
#else
  return a;
#endif
}
