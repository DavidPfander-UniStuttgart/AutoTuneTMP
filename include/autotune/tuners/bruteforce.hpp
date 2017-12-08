#pragma once

#include "../autotune.hpp"
#include "../parameter.hpp"
#include "common.hpp"
#include "countable_set.hpp"

namespace autotune {

template <class... T> class kernel;

namespace tuners {

template <class... Args> class bruteforce;

template <typename R, typename... Args>
class bruteforce<autotune::kernel<R, cppjit::detail::pack<Args...>>>
    : public abstract_tuner<countable_set, R, Args...> {
private:
  autotune::kernel<R, cppjit::detail::pack<Args...>> &f;
  bool verbose;
  countable_set &parameters;

public:
  bruteforce(autotune::kernel<R, cppjit::detail::pack<Args...>> &f,
             countable_set &parameters)
      : f(f), verbose(false), parameters(parameters) {}

  countable_set tune(Args &... args) {
    bool is_valid = true;

    f.write_header();

    double total_combinations = 1.0;
    for (size_t i = 0; i < parameters.size(); i++) {
      total_combinations *= parameters[i]->count_values();
    }

    if (verbose) {
      std::cout << "total combinations to test: " << total_combinations
                << std::endl;
    }

    parameter_value_set original_values = f.get_parameter_values();

    // brute-force tuner
    for (size_t i = 0; i < parameters.size(); i++) {
      parameters[i]->set_min();
    }

    // evaluate initial vector, always valid
    size_t combination_counter = 1;
    if (verbose) {
      std::cout << "evaluating combination " << combination_counter
                << " (out of " << total_combinations << ")" << std::endl;
      std::cout << "current attempt:" << std::endl;
      f.print_values();
    }
    combination_counter += 1;
    bool first = true;

    double optimal_duration = this->evaluate(is_valid, parameters, f, args...);
    countable_set optimal_parameters;
    if (is_valid) {
      first = false;
      optimal_parameters = parameters.clone();
      this->report_verbose("new best kernel", optimal_duration, f);
    }

    size_t current_index = 0;
    while (true) {
      // left the range of valid indices, done!
      if (current_index == parameters.size()) {
        break;
      }

      // the is another value for the current parameter
      if (parameters[current_index]->next()) {
        // reset the parameters "below" and start with the first parameter
        // again
        for (size_t i = 0; i < current_index; i++) {
          parameters[i]->set_min();
        }
        current_index = 0;

        // evaluate new valid value vector
        if (verbose) {
          std::cout << "evaluating combination " << combination_counter
                    << " (out of " << total_combinations << ")" << std::endl;
          std::cout << "current attempt:" << std::endl;
          f.print_values();
        }
        combination_counter += 1;
        double duration = this->evaluate(is_valid, parameters, f, args...);
        if (is_valid && (first || duration < optimal_duration)) {
          first = false;
          optimal_duration = duration;
          optimal_parameters = parameters.clone();
          this->report_verbose("new best kernel", optimal_duration, f);
        }

      } else {
        // no valid more values, try next parameter "above"
        current_index += 1;
      }
    }

    f.set_parameter_values(original_values);
    return optimal_parameters;
  }

  void set_verbose(bool verbose) { this->verbose = verbose; }
};
} // namespace tuners
} // namespace autotune
