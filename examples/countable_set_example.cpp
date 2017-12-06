#include <algorithm>
#include <iostream>
#include <vector>

#include "autotune/parameter.hpp"
#include "autotune/tuners/countable_set.hpp"

int main(void) {
  std::cout << "abccf" << std::endl;

  autotune::countable_set parameters;
  // std::shared_ptr<autotune::fixed_set_parameter> p =
  //     std::make_shared<autotune::fixed_set_parameter>("PAR_1",
  //                                                     {"eins", "zwei",
  //                                                     "drei"});
  std::shared_ptr<autotune::fixed_set_parameter> p =
      std::shared_ptr<autotune::fixed_set_parameter>(
          new autotune::fixed_set_parameter("PAR_1", {"eins", "zwei", "drei"}));
  parameters.add_countable_parameter(p);

  return 0;
}
