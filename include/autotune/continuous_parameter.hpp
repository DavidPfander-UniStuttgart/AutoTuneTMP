#pragma once

#include <cmath>
#include <functional>
#include <iomanip>

namespace autotune {

class stepable_continuous_parameter {
protected:
  std::string name;
  double initial;
  double current;
  double step;
  std::function<double(double, double)> next_functional;
  std::function<double(double, double)> prev_functional;

public:
  stepable_continuous_parameter(
      const std::string &name, double initial, double step,
      std::function<double(double, double)> next_functional =
          std::plus<double>(),
      std::function<double(double, double)> prev_functional =
          std::minus<double>())
      : name(name), initial(initial), current(initial), step(step),
        next_functional(next_functional), prev_functional(prev_functional) {}

  const std::string &get_name() const { return this->name; }

  virtual const std::string get_value() const {
    std::stringstream ss;
    ss << std::fixed << current;
    std::string str(ss.str());
    str.erase(str.find_last_not_of('0') + 1, std::string::npos);
    if (*(str.end() - 1) == '.') {
      str.erase(str.end() - 1, str.end());
    }
    return str;
  }

  virtual void set_initial() {
    // TODO: should be extended, so that an initial guess can be supplied
    current = initial;
  }

  virtual bool next() {
    // current += step;
    current = next_functional(current, step);
    return true;
  }

  virtual bool prev() {
    // current -= step;
    current = prev_functional(current, step);
    return true;
  }
};

class countable_continuous_parameter : public stepable_continuous_parameter {
private:
  double min;
  double max;

public:
  countable_continuous_parameter(
      const std::string &name, double initial, double step, double min,
      double max, std::function<double(double, double)> next_functional =
                      std::plus<double>(),
      std::function<double(double, double)> prev_functional =
          std::minus<double>())
      : stepable_continuous_parameter(name, initial, step, next_functional,
                                      prev_functional),
        min(min), max(max) {}

  virtual bool next() override {
    // if (this->current + this->step <= max) {
    if (next_functional(current, step) <= max) {
      // this->current += this->step;
      current = next_functional(current, step);
      return true;
    }
    return false;
  }

  virtual bool prev() override {
    // if (this->current - this->step >= min) {
    if (prev_functional(current, step) >= min) {
      // this->current -= this->step;
      current = prev_functional(current, step);
      return true;
    }
    return false;
  }

  void set_min() { this->current = min; }

  void set_max() { this->current = max; }

  double get_min() const { return min; }

  double get_max() const { return min; }

  size_t count_values() const {
    // TODO: implement
    return static_cast<size_t>(std::floor((max - min) / this->step)) + 1;
  }
};

class limited_continuous_parameter {
private:
  std::string name;
  double initial;
  double current;
  double min;
  double max;
  bool integer_parameter;

public:
  limited_continuous_parameter(const std::string &name, double initial,
                               double min, double max,
                               bool integer_parameter = false)
      : name(name), initial(initial), current(initial), min(min), max(max),
        integer_parameter(integer_parameter) {}

  const std::string &get_name() const { return this->name; }

  virtual const std::string get_value() const {
    std::stringstream ss;
    ss << std::fixed << current;
    std::string str(ss.str());
    str.erase(str.find_last_not_of('0') + 1, std::string::npos);
    if (*(str.end() - 1) == '.') {
      str.erase(str.end() - 1, str.end());
    }
    return std::to_string(current);
  }

  void set_min() { current = min; }

  void set_max() { current = max; }

  double get_min() const { return min; }

  double get_max() const { return max; }

  virtual void set_initial() {
    // TODO: should be extended, so that an initial guess can be supplied
    current = initial;
  }

  bool set_value(double new_value) {
    if (new_value < min || new_value > max) {
      return false;
    }
    if (integer_parameter && (new_value != std::trunc(new_value))) {
      return false;
    }
    current = new_value;
    return true;
  }

  bool is_integer_parameter() const { return integer_parameter; }
};

} // namespace autotune
