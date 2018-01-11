#pragma once

#include "util.hpp"
#include <cmath>
#include <functional>
#include <iomanip>
#include <random>

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

  const std::string get_value() const {
    return detail::truncate_trailing_zeros(current);
  }

  void set_initial() {
    // TODO: should be extended, so that an initial guess can be supplied
    current = initial;
  }

  bool next() {
    // current += step;
    current = next_functional(current, step);
    return true;
  }

  bool prev() {
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

  bool next() {
    // if (this->current + this->step <= max) {
    if (next_functional(current, step) <= max) {
      // this->current += this->step;
      current = next_functional(current, step);
      return true;
    }
    return false;
  }

  bool prev() {
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

  void set_random_value() {
    size_t num_values = count_values();

    std::uniform_real_distribution<double> distribution(0, num_values - 1);
    std::random_device rd;
    std::default_random_engine generator(rd());

    size_t value_index = distribution(generator);

    double value = get_min();
    for (size_t i = 0; i < value_index; i++) {
      value = next_functional(value, step);
    }

    current = value;
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

  const std::string get_value() const {
    return detail::truncate_trailing_zeros(current);
  }

  void set_min() { current = min; }

  void set_max() { current = max; }

  double get_min() const { return min; }

  double get_max() const { return max; }

  void set_initial() {
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

  void set_random_value() {
    if (this->is_integer_parameter()) {
      // randomize index
      std::uniform_int_distribution<size_t> distribution(
          static_cast<size_t>(min), static_cast<size_t>(max));
      std::random_device rd;
      std::default_random_engine generator(rd());
      current = distribution(generator);
    } else {
      std::uniform_real_distribution<double> distribution(min, max);
      std::random_device rd;
      std::default_random_engine generator(rd());
      current = distribution(generator);
    }
  }
};

} // namespace autotune
