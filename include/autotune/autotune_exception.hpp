#pragma once

#include <exception>

namespace autotune {

class autotune_exception : public std::exception {
public:
  explicit autotune_exception(const std::string &what_arg)
      : what_arg(what_arg) {}

  virtual const char *what() const throw() override { return what_arg.c_str(); }

private:
  std::string what_arg;
};
}
