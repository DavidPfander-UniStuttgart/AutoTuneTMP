#pragma once

#include "autotune_exception.hpp"
#include "util.hpp"
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
  bool multiply;
  std::function<double(double, double)> next_functional;
  std::function<double(double, double)> prev_functional;

public:
  stepable_continuous_parameter(const std::string &name, double initial,
                                double step, bool multiply = false)
      : name(name), initial(initial), current(initial), step(step),
        multiply(multiply) {
    if (multiply) {
      next_functional = std::multiplies<double>();
      prev_functional = std::divides<double>();
    } else {
      next_functional = std::plus<double>();
      prev_functional = std::minus<double>();
    }
  }

  const std::string &get_name() const { return this->name; }

  const std::string get_value() const {
    return detail::truncate_trailing_zeros(current);
  }

  double get_raw_value() const { return current; }

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

  void to_nearest_valid(double factor) {
    if (!multiply) {
      current = autotune::detail::round_to_nearest(current, factor);
    } else {
      throw autotune_exception(
          "cannot adjust to nearest valid value with multiplied steps");
    }
  }

  double get_step() { return step; }
};

class countable_continuous_parameter : public stepable_continuous_parameter {
private:
  double min;
  double max;

public:
  countable_continuous_parameter(const std::string &name, double initial,
                                 double step, double min, double max,
                                 bool multiply = false)
      : stepable_continuous_parameter(name, initial, step, multiply), min(min),
        max(max) {}

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
    // std::cout << "current: " << current << " step: " << step << " min: " <<
    // min
    //           << std::endl;
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

  double get_max() const { return max; }

  size_t count_values() const {
    if (multiply) {
      double range = max / min;
      return std::log2(range) / std::log2(step) + 1;
    } else {
      return static_cast<size_t>(std::floor((max - min) / this->step)) + 1;
    }
  }

  void set_random_value() {
    size_t num_values = count_values();
    auto random_gen = detail::make_uniform_int_generator(0ul, num_values - 1ul);

    size_t value_index = random_gen();

    double value = get_min();
    for (size_t i = 0; i < value_index; i++) {
      value = next_functional(value, step);
    }

    current = value;
  }

  void set_value_unsafe(const std::string &v) { current = std::stod(v); }

  void to_nearest_valid(double factor) {
    if (!multiply) {
      current =
          autotune::detail::round_to_nearest_bounded(current, factor, min, max);
    } else {
      throw autotune_exception(
          "cannot adjust to nearest valid value with multiplied steps");
    }
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

  double get_raw_value() const { return current; }

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

  void set_value_unsafe(const std::string &v) { current = std::stod(v); }

  void to_nearest_valid(double factor) {
    current =
        autotune::detail::round_to_nearest_bounded(current, factor, min, max);
  }
};

} // namespace autotune
