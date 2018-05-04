#include <algorithm>
#include <functional>
#include <vector>

#include "parameters.hpp"

using namespace std;

AUTOTUNE_EXPORT void square(vector<double> &origin, vector<double> &dest) {
  transform(origin.begin(), origin.end(), origin.begin(), dest.begin(),
            multiplies<double>());
}
