#include <algorithm>
#include <functional>
#include <vector>

using namespace std;

extern "C" void square(vector<double> &origin, vector<double> &dest) {
  transform(origin.begin(), origin.end(), origin.begin(), dest.begin(),
            multiplies<double>());
}
