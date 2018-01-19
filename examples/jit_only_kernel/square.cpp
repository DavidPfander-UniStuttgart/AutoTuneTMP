#include "parameters.hpp"

#include <algorithm>
#include <functional>
#include <vector>

extern "C" void square(std::vector<double> &origin, std::vector<double> &dest) {
  std::transform(origin.begin(), origin.end(), origin.begin(), dest.begin(),
                 std::multiplies<double>());
}
