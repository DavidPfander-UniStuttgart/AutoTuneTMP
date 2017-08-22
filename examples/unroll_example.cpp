#include "opttmp/loop/unroll_loop.hpp"

#include <iostream>

template <size_t i> void myfunction(int &test) {
  std::cout << "test: " << test << " i: " << i << std::endl;
  test = 4;
}

// need macro, because templates can not be passed as arguments
// defines struct myfunction_body that can be used as a loop body
// with template index parameter
DEFINE_LOOP_BODY(myfunction)

int main(void) {
  int test_arg = 5;
  std::cout << "arg before unrolled loop, test_arg: " << test_arg << std::endl;
  opttmp::loop::unroll_loop_template<0, 10, 1>(myfunction_body(test_arg));
  std::cout << "arg after unrolled loop, test_arg: " << test_arg << std::endl;
  return 0;
}
