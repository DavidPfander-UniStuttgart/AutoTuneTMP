#pragma once

#include "parameter.hpp"

namespace autotune {
namespace factory {

std::shared_ptr<continuous_parameter>
make_continuous_parameter(const std::string &name, double initial,
                          double step) {
  std::shared_ptr<stepable_continuous_parameter> p =
      std::make_shared<stepable_continuous_parameter>(name, initial);
  p->set_next_function(stepper(step));
  p->set_prev_function(stepper(-1.0 * step));

  return p;
}

std::shared_ptr<continuous_parameter>
make_continuous_parameter(const std::string &name, double initial) {
  std::shared_ptr<stepable_continuous_parameter> p =
      std::make_shared<stepable_continuous_parameter>(name, initial);
  p->set_next_function(stepper(1.0));
  p->set_prev_function(stepper(-1.0));

  return p;
}

std::shared_ptr<continuous_parameter>
make_continuous_parameter(const std::string &name, double initial, double min,
                          double max, double step) {
  std::shared_ptr<countable_continuous_parameter> p =
      std::make_shared<countable_continuous_parameter>(name, initial);
  p->set_next_function(stepper(step));
  p->set_prev_function(stepper(-1.0 * step));
  p->set_valid_function([=](double value) {
    if (value >= min && value <= max) {
      return true;
    } else {
      return false;
    }
  });

  return p;
}

std::shared_ptr<continuous_parameter>
make_continuous_parameter(const std::string &name, double initial, double min,
                          double max) {
  std::shared_ptr<countable_continuous_parameter> p =
      std::make_shared<countable_continuous_parameter>(name, initial);
  p->set_next_function(stepper(1.0));
  p->set_prev_function(stepper(-1.0));
  p->set_valid_function([=](double value) {
    if (value >= min && value <= max) {
      return true;
    } else {
      return false;
    }
  });

  return p;
}

} // namespace factory
} // namespace autotune

