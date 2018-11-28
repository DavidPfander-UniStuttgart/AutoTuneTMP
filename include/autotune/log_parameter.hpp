#pragma once

#include "util.hpp"

namespace autotune {
  class log_parameter {
  private:
    std::string name;
    double initial;
    double current;
    double base;
    double min;
    double max;
    double step;
  public:
    log_parameter(const std::string &name, double initial,
		  double base, double min, double max, double step): name(name), initial(initial), current(initial), base(base), min(min), max(max), step(step)  {}

    const std::string &get_name() const { return this->name; }

    const std::string get_value() const {
      return std::to_string(static_cast<uint64_t>(std::pow(base, current)));
    }

    double get_raw_value() const { return pow(base, current); }

    void set_initial() {
      current = initial;
    }

    void set_min() {
      current = min;
    }

    bool next() {
      if (current + step <= max && current + step >= min) {
	current += step;
	return true;
      } else {
	return false;
      }      
    }

    bool prev() {
      if (current - step <= max && current - step >= min) {
	current -= step;
	return true;
      } else {
	return false;
      }      
    }

    double get_step() { return step; }

    size_t count_values() const {
      return static_cast<uint64_t>(max - min / step);
    }

    // only produces integer values
    void set_random_value() {
      auto random_gen = detail::make_uniform_int_generator(min, max - 1.0);
      current = static_cast<double>(random_gen());
    };
    
    void set_value_unsafe(const std::string &v) {
      current = std::stod(v);
    };
  };
}
