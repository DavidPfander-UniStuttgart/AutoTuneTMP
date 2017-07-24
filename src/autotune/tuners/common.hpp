#pragma once

#include <chrono>

namespace autotune {

// setup parameters, compile and measure duration of parameter combination
template <class F, typename... Args>
double evaluate(const std::vector<size_t> &indices, F f, Args... args) {

  std::cout << "----------------- eval begin ----------------" << std::endl;

  f->create_parameter_file(indices);

  f->compile();

  auto start = std::chrono::high_resolution_clock::now();

  // call kernel, discard possibly returned values
  (*f)(args...);

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration = end - start;

  std::cout << "----------------- eval end ----------------" << std::endl;

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
