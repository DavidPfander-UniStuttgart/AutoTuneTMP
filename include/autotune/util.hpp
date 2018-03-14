#pragma once

#include <sstream>
#include <tuple>

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
  // std::cout << "value: " << value << " factor: " << factor
  //          << " remainder: " << remainder << std::endl;
  int64_t lower = value - remainder;
  int64_t upper = value + (factor - remainder);

  if (2 * remainder <= factor) {
    if (static_cast<double>(lower) >= min &&
        static_cast<double>(lower) <= max) {
      return lower;
    } else {
      return min;
    }
  } else {
    if (static_cast<double>(upper) >= min &&
        static_cast<double>(upper) <= max) {
      return upper;
    } else {
      return max;
    }
  }
}

template <size_t i = 0, typename F, typename... Us>
void iterate_tuple(std::tuple<Us...> &t, F f) {
  if
    constexpr(i < sizeof...(Us)) {
      f(std::get<i>(t));
      iterate_tuple<i + 1>(t, f);
    }
  else {
    // to silence unused parameter warning in last instantiation
    (void)f;
  }
}
}
}
