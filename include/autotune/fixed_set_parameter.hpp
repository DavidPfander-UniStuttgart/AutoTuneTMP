#pragma once

#include <memory>
#include <vector>

#include "abstract_parameter.hpp"
#include "parameter_set.hpp"

namespace autotune {

template <typename T> class fixed_set_parameter : public abstract_parameter {
private:
  size_t cur_index;
  std::vector<std::string> values;

public:
  fixed_set_parameter(const std::string &name, const std::vector<T> &values)
      : abstract_parameter(name), cur_index(0), values(values) {}

  const std::vector<std::string> &get_values() const { return this->values; }

  void set_index(size_t new_index) { cur_index = new_index; };

  virtual const std::string get_value() const override {
    return this->values[cur_index];
  }

  size_t count_values() const { return values.size(); }

  bool next() {
    if (cur_index + 1 < values.size()) {
      cur_index += 1;
      return true;
    } else {
      return false;
    }
  }

  bool prev() {
    if (cur_index > 0) {
      cur_index -= 1;
      return true;
    } else {
      return false;
    }
  }

  void set_min() { cur_index = 0; };

  virtual void set_initial() {
    // TODO: should be extended, so that an initial guess can be supplied
    cur_index = 0;
  }
};
}
