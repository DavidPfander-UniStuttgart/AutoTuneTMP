#pragma once

#include <memory>
#include <vector>

#include "parameter_value_set.hpp"

namespace autotune {

template <typename T> class fixed_set_parameter {
private:
  std::string name;
  size_t cur_index;
  std::vector<T> values;

public:
  fixed_set_parameter(const std::string &name, const std::vector<T> &values)
      : name(name), cur_index(0), values(values) {}

  fixed_set_parameter(const fixed_set_parameter<T> &other)
      : name(other.name), cur_index(other.cur_index), values(other.values) {}

  const std::string &get_name() const { return this->name; }

  const std::vector<std::string> &get_values() const { return this->values; }

  void set_index(size_t new_index) { cur_index = new_index; };

  const std::string get_value() const {
    if
      constexpr(std::is_same<T, bool>::value) {
        if (this->values[cur_index]) {
          return "true";
        } else {
          return "false";
        }
      }
    else {
      return std::to_string(this->values[cur_index]);
    }
  }

  T get_raw_value() { return values[cur_index]; };

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

  void set_random_value() {
    auto random_gen =
        detail::make_uniform_int_generator(0ul, this->values.size() - 1ul);
    cur_index = random_gen();
  }

  void set_value_unsafe(const std::string &v) {
    this->set_min();
    while (true) {
      if (this->get_value().compare(v) == 0) {
        break;
      }
      if (!this->next()) {
        throw;
      }
    }
  }
};

template <> class fixed_set_parameter<std::string> {
private:
  std::string name;
  size_t cur_index;
  std::vector<std::string> values;
  bool quote_string;

public:
  fixed_set_parameter(const std::string &name,
                      const std::vector<std::string> &values,
                      bool quote_string = true)
      : name(name), cur_index(0), values(values), quote_string(quote_string) {}

  const std::string &get_name() const { return this->name; }

  const std::vector<std::string> &get_values() const { return this->values; }

  void set_index(size_t new_index) { cur_index = new_index; };

  const std::string get_value() const {
    if (quote_string) {
      return std::string("\"") + this->values[cur_index] + std::string("\"");
    } else {
      return this->values[cur_index];
    }
  }

  std::string get_raw_value() { return values[cur_index]; };

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

  void set_quote_string(bool quote_string) {
    this->quote_string = quote_string;
  }

  void set_random_value() {
    // randomize index
    std::uniform_int_distribution<size_t> distribution(0,
                                                       this->values.size() - 1);
    std::random_device rd;
    std::default_random_engine generator(rd());
    cur_index = distribution(generator);
  }

  void set_value_unsafe(const std::string &v) {
    this->set_min();
    while (true) {
      if (this->get_value().compare(v) == 0) {
        break;
      }
      if (!this->next()) {
        throw;
      }
    }
  }
};
}
