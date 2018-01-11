#pragma once

#include <sstream>

namespace autotune {
namespace detail {

std::string truncate_trailing_zeros(double value) {
  std::stringstream ss;
  ss << std::fixed << value;
  std::string str(ss.str());
  str.erase(str.find_last_not_of('0') + 1, std::string::npos);
  if (*(str.end() - 1) == '.') {
    str.erase(str.end() - 1, str.end());
  }
  return str;
}
}
}
