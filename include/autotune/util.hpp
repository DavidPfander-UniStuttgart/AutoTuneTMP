#pragma once

#include <sstream>

namespace autotune {
namespace detail {

inline std::string truncate_trailing_zeros(double value) {
  std::stringstream ss;
  ss << std::fixed << value;
  std::string str(ss.str());
  str.erase(str.find_last_not_of('0') + 1, std::string::npos);
  if (*(str.end() - 1) == '.') {
    str.erase(str.end() - 1, str.end());
  }
  return str;
}

inline int64_t round_to_nearest(int64_t value, int64_t factor) {
  int64_t remainder = value % factor;

  if (static_cast<double>(remainder) / static_cast<double>(factor) <=
      0.5 * static_cast<double>(factor)) {
    return value - remainder;
  } else {
    return value + (factor - remainder);
  }
}

inline int64_t round_to_nearest_bounded(int64_t value, int64_t factor,
                                        double min, double max) {
  int64_t remainder = value % factor;
  //std::cout << "value: " << value << " factor: " << factor
  //          << " remainder: " << remainder << std::endl;
  int64_t lower = value - remainder;
  int64_t upper = value + (factor - remainder);

  if (static_cast<double>(remainder) / static_cast<double>(factor) <=
      0.5 * static_cast<double>(factor)) {
    if (lower >= min && lower <= max) {
      return lower;
    } else {
      return min;
    }
  } else {
    if (lower >= min && lower <= max) {
      return upper;
    } else {
      return max;
    }
  }
}
}
}
