#pragma once

#include <memory>
#include <vector>

#include "abstract_parameter.hpp"
#include "parameter_value_set.hpp"

namespace autotune {

namespace detail {
class is_quotable {
protected:
  bool quote_string;

public:
  is_quotable() : quote_string(false) {}
};

class is_not_quotable {};
}

template <typename T>
class constant_parameter
    : public std::conditional<!std::is_same<T, std::string>::value,
                              detail::is_quotable,
                              detail::is_not_quotable>::type {
private:
  std::string name;
  T value;

public:
  constant_parameter(const std::string &name, const T &value)
      : name(name), value(value) {}

  // only available for T == std::string
  constant_parameter(
      const std::string &name, const T &value,
      typename std::enable_if<std::is_same<T, std::string>::value, bool>::type
          quote_string)
      : name(name), value(value) {
    this->quote_string = quote_string;
  }

  const std::string &get_name() const { return this->name; }

  virtual const std::string get_value() const {
    if
      constexpr(std::is_same<T, bool>::value) {
        if (value) {
          return "true";
        } else {
          return "false";
        }
      }
    else if
      constexpr(std::is_same<T, std::string>::value) {
        if (this->quote_string) {
          return std::string("\"") + value + std::string("\"");
        } else {
          return value;
        }
      }
    else {
      return std::to_string(value);
    }
  }

  size_t count_values() const { return 1; }

  bool next() { return false; }

  bool prev() { return false; }

  void set_min(){};

  void set_max(){};

  virtual void set_initial() {}
};
}
