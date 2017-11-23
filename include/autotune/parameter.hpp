#pragma once

#include <vector>

namespace autotune {

class tunable_parameter {
private:
  std::string name;
  std::vector<std::string> values;

public:
  tunable_parameter(const std::string &name,
                    const std::vector<std::string> &values)
      : name(name), values(values) {}

  const std::string &get_name() const { return this->name; }

  const std::vector<std::string> &get_values() const { return this->values; }

  const std::string &get_value(size_t i) const { return this->values[i]; }

  size_t size() { return values.size(); }

  std::string to_parameter_source_line(size_t value_index) {
    return "#define " + name + " " + values[value_index] + "\n";
  }
};
}
