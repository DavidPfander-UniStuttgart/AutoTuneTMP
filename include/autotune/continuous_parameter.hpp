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
  // std::function<double(double, double)> next_functional;
  // std::function<double(double, double)> prev_functional;

public:
  stepable_continuous_parameter(const std::string &name, double initial,
                                double step, bool multiply = false)
      : name(name), initial(initial), current(initial), step(step),
        multiply(multiply) {
    if (multiply) {
      initial = initial / step; // step is factor for multiply
      current = initial;
    }
  }

  const std::string &get_name() const { return this->name; }

  const std::string get_value() const {
    if (multiply) {
      return detail::truncate_trailing_zeros(current * step);
    } else {
      return detail::truncate_trailing_zeros(current);
    }
  }

  double get_raw_value() const {
    if (multiply) {
      return current * step;
    } else {
      return current;
    }
  }

  void set_initial() {
    // TODO: should be extended, so that an initial guess can be supplied
    current = initial;
  }

  bool next() {
    if (multiply) {
      current += 1;
    } else {
      current += step;
    }
    return true;
  }

  bool prev() {
    if (multiply) {
      current -= 1;
    } else {
      current -= step;
    }
    return true;
  }

  void to_nearest_valid(double factor) {
    if (!multiply) {
      current = autotune::detail::round_to_nearest(current, factor);
    } else {
      current =
          autotune::detail::round_to_nearest(current * step, factor) / step;
    }
  }

  double get_step() { return step; }
};

class countable_continuous_parameter : public stepable_continuous_parameter {
private:
  double min;
  double max;
  size_t value_range;

public:
  countable_continuous_parameter(const std::string &name, double initial,
                                 double step, double min, double max,
                                 bool multiply = false)
      : stepable_continuous_parameter(name, initial, step, multiply), min(min),
        max(max), value_range(1) {
    set_min();
    while (next()) {
      value_range += 1;
    }
    set_initial();
  }

  bool next() {
    if (multiply) {
      if ((current + 1.0) * step > max) {
        return false;
      }
    } else {
      if (current + step > max) {
        return false;
      }
    }
    return stepable_continuous_parameter::next();
  }

  bool prev() {
    if (multiply) {
      if ((current - 1.0) * step < min) {
        return false;
      }
    } else {
      if (current - step < min) {
        return false;
      }
    }
    return stepable_continuous_parameter::prev();
  }

  void set_min() {
    if (multiply) {
      this->current = min / step;
    } else {
      this->current = min;
    }
  }

  void set_max() {
    if (multiply) {
      this->current = max / step;
    } else {
      this->current = max;
    }
  }

  double get_min() const { return min; }

  double get_max() const { return max; }

  size_t count_values() const { return value_range; }

  void set_random_value() {
    size_t num_values = count_values();
    auto random_gen = detail::make_uniform_int_generator(0ul, num_values - 1ul);

    size_t value_index = random_gen();

    set_min();
    for (size_t i = 0; i < value_index; i += 1) {
      next();
    }
  }

  void set_value_unsafe(const std::string &v) { current = std::stod(v); }

  void to_nearest_valid(double factor) {
    if (!multiply) {
      current =
          autotune::detail::round_to_nearest_bounded(current, factor, min, max);
    } else {
      current = autotune::detail::round_to_nearest_bounded(current * step,
                                                           factor, min, max) /
                step;
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
