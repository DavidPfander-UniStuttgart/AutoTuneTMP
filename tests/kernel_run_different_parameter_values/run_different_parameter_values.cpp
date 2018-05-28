#include "autotune_kernel.hpp"

AUTOTUNE_EXPORT int run_different_parameter_values(int a) {
#if PAR_1 == 1
  return 1;
#else
  return 2;
#endif
}
