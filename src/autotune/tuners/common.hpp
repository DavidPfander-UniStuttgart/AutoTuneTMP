#pragma once

#include <chrono>

namespace autotune {

// setup parameters, compile and measure duration of parameter combination
template <class F, class test, typename... Args>
double evaluate(const std::vector<size_t> &indices, F f, test t, Args... args) {

  if (f->is_verbose()) {
    std::cout << "------ begin eval ------" << std::endl;
    f->print_values(indices);
  }

  f->create_parameter_file(indices);

  f->compile();

  if (!f->is_valid_parameter_combination()) {
    if (f->is_verbose()) {
      std::cout << "invalid parameter combination encountered" << std::endl;
    }
    return std::numeric_limits<double>::max();
  }

  auto start = std::chrono::high_resolution_clock::now();

  // call kernel, discard possibly returned values
  bool test_ok = t((*f)(args...));
  if (!test_ok) {
    if (f->is_verbose()) {
      std::cout << "warning: test for combination failed!" << std::endl;
    }
    return std::numeric_limits<double>::max();
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration = end - start;

  if (f->is_verbose()) {
    std::cout << "------- end eval -------" << std::endl;
  }

  return duration.count();
}

template <class F>
void report(const std::string &message, double duration,
            std::vector<size_t> &indices, F f) {
  std::vector<tunable_parameter> &parameters = f->get_parameters();
  std::cout << message << "; duration: " << duration;
  std::cout << "; values: ";
  for (size_t i = 0; i < parameters.size(); i++) {
    if (i > 0) {
      std::cout << ", ";
    }
    std::cout << parameters[i].get_value(indices[i]);
  }
  std::cout << std::endl;
}

template <class F>
void report_verbose(const std::string &message, double duration,
                    std::vector<size_t> &indices, F f) {
  if (f->is_verbose()) {
    report(message, duration, indices, f);
  }
}
}
