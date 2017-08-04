#pragma once

#include "../parameter.hpp"
#include "common.hpp"

namespace autotune {

template <class F, class test, typename... Args>
std::vector<size_t> bruteforce(F f, test t, const Args &... args) {
  std::vector<tunable_parameter> &parameters = f->get_parameters();

  if (f->is_verbose()) {
    double total_combinations = 1.0;
    for (size_t i = 0; i < parameters.size(); i++) {
      total_combinations *= parameters[i].get_values().size();
    }
    std::cout << "total combinations to test: " << total_combinations
              << std::endl;
  }

  // brute-force tuner
  std::vector<std::string> values(parameters.size());
  for (size_t i = 0; i < parameters.size(); i++) {
    values[i] = parameters[i].get_value(0);
  }
  std::vector<size_t> indices(parameters.size(), 0);
  std::vector<size_t> optimal_indices(parameters.size(), 0);
  // evaluate initial vector, always valid
  // f->print_values(values);
  double optimal_duration = evaluate(indices, f, t, args...);
  std::copy(indices.begin(), indices.end(), optimal_indices.begin());

  report_verbose("new best kernel", optimal_duration, optimal_indices, f);

  size_t current_index = 0;
  while (true) {
    // left the range of valid indices, done!
    if (current_index == parameters.size()) {
      break;
    }

    // the is another value for the current parameter
    if (indices[current_index] + 1 < parameters[current_index].size()) {
      values[current_index] =
          parameters[current_index].get_value(indices[current_index] + 1);
      indices[current_index] += 1;
      // reset the parameters "below" and start with the first parameter again
      for (size_t i = 0; i < current_index; i++) {
        values[i] = parameters[i].get_value(0);
        indices[i] = 0;
      }
      current_index = 0;

      // evaluate new valid value vector
      double duration = evaluate(indices, f, t, args...);
      if (duration < optimal_duration) {
        std::copy(indices.begin(), indices.end(), optimal_indices.begin());
        optimal_duration = duration;
        report_verbose("new best kernel", optimal_duration, optimal_indices, f);
      }

    } else {
      // no valid more values, try next parameter "above"
      current_index += 1;
    }
  }
  return optimal_indices;
}
}
