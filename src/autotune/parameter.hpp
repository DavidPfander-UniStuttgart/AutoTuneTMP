#pragma once

#include <vector>

namespace autotune {

class tunable_parameter {
private:
  std::string name;
  std::vector<std::string> values;

public:
  tunable_parameter(std::string name, std::vector<std::string> values)
      : name(name), values(values) {}

  const std::string &get_name() const { return this->name; }

  const std::vector<std::string> &get_values() const { return this->values; }
};
}
