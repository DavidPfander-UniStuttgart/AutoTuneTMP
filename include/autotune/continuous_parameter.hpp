#pragma once

#include <cmath>
#include <functional>
#include <iomanip>

namespace autotune {

class continuous_parameter : public abstract_parameter {
protected:
  double current;
  double initial;

public:
  continuous_parameter(const std::string &name, double initial)
      : abstract_parameter(name), current(initial), initial(initial) {}

  // TODO: probably need a set_value method

  virtual const std::string get_value() const override {
    return std::to_string(current);
  }

  virtual void set_initial() {
    // TODO: should be extended, so that an initial guess can be supplied
    current = initial;
  }

  // virtual std::string to_parameter_source_line() override {
  //   return "#define " + name + " " + std::to_string(current) + "\n";
  // }

  // virtual std::shared_ptr<continuous_parameter> clone() override {
  //   auto new_instance =
  //       std::make_shared<continuous_parameter>(this->name, initial);
  //   new_instance->current = this->current;
  //   return new_instance;
  // }
};

class stepable_continuous_parameter : public continuous_parameter {
protected:
  double step;

  // // modifies the current value to a next value
  // // this depends on the parameter, could be adding a increment, double
  // // something more complex
  // std::function<double(double)> next_function;
  // // modifies the current to a previous value (opposite to next)
  // std::function<double(double)> prev_function;
  // // validates values, used by the next and prev functions
  // // can be used to implement lower and upper bounds
  // std::function<bool(double)> valid_function;

public:
  stepable_continuous_parameter(const std::string &name, double initial,
                                double step)
      : continuous_parameter(name, initial), step(step) {}

  // void set_next_function(std::function<double(double)> next_function) {
  //   this->next_function = next_function;
  // }

  virtual bool next() {
    current += step;
    return true;
  }

  virtual bool prev() {
    current -= step;
    return true;
  }

  // virtual std::shared_ptr<abstract_parameter> clone() override {
  //   std::shared_ptr<continuous_parameter> new_instance =
  //       std::make_shared<continuous_parameter>(this->name, initial);
  //   new_instance->current = this->current;
  //   return std::dynamic_pointer_cast<abstract_parameter>(new_instance);
  // }

  // bool next() {
  //   double temp = this->next_function(current);
  //   if (valid_function) {
  //     if (valid_function(temp)) {
  //       current = temp;
  //       return true;
  //     } else {
  //       return false;
  //     }
  //   } else {
  //     current = temp;
  //     return true;
  //   }
  // }

  // void set_prev_function(std::function<double(double)> prev_function) {
  //   this->prev_function = prev_function;
  // }

  // bool prev() {
  //   double temp = this->prev_function(current);
  //   if (valid_function) {
  //     if (valid_function(temp)) {
  //       current = temp;
  //       return true;
  //     } else {
  //       return false;
  //     }
  //   } else {
  //     current = temp;
  //     return true;
  //   }
  // }

  // void set_valid_function(std::function<double(double)> valid_function) {
  //   this->valid_function = valid_function;
  // }
};

class countable_continuous_parameter : public stepable_continuous_parameter {
private:
  double min;
  double max;

public:
  countable_continuous_parameter(const std::string &name, double initial,
                                 double step, double min, double max)
      : stepable_continuous_parameter(name, initial, step), min(min), max(max) {
  }

  virtual bool next() override {
    if (current + step <= max) {
      current += step;
      return true;
    }
    return false;
  }

  virtual bool prev() override {
    if (current - step >= min) {
      current -= step;
      return true;
    }
    return false;
  }

  void set_min() { current = min; }

  void set_max() { current = max; }

  double get_min() const { return min; }

  double get_max() const { return min; }

  size_t count_values() const {
    // TODO: implement
    return static_cast<size_t>(std::floor((max - min) / step)) + 1;
  }
};

class limited_continuous_parameter : public continuous_parameter {
private:
  double min;
  double max;
  bool integer_parameter;

public:
  limited_continuous_parameter(const std::string &name, double initial,
                               double min, double max,
                               bool integer_parameter = false)
      : continuous_parameter(name, initial), min(min), max(max),
        integer_parameter(integer_parameter) {}

  void set_min() { current = min; }

  void set_max() { current = max; }

  double get_min() const { return min; }

  double get_max() const { return max; }

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
