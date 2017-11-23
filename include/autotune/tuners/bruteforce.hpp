#pragma once

#include "../autotune.hpp"
#include "../parameter.hpp"
#include "common.hpp"

namespace autotune {

template <class... T> class kernel;

namespace tuners {

template <class... Args> class bruteforce;

template <typename R, typename... Args>
class bruteforce<autotune::kernel<R, cppjit::detail::pack<Args...>>>
    : public abstract_tuner<R, Args...> {
private:
  autotune::kernel<R, cppjit::detail::pack<Args...>> &f;
  std::function<bool(const R &)> &t;

public:
  bruteforce(autotune::kernel<R, cppjit::detail::pack<Args...>> &f,
             std::function<bool(const R &)> &t)
      : f(f), t(t) {}

  std::vector<size_t> tune(Args &... args) {
    std::vector<tunable_parameter> &parameters = f.get_parameters();
    bool is_valid = true;

    f.write_header();

    double total_combinations = 1.0;
    for (size_t i = 0; i < parameters.size(); i++) {
      total_combinations *= parameters[i].get_values().size();
    }

    size_t combination_counter = 1;

    if (f.is_verbose()) {
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
    // f.print_values(values);
    if (f.is_verbose()) {
      std::cout << "evaluating combination " << combination_counter
                << " (out of " << total_combinations << ")" << std::endl;
    }
    combination_counter += 1;
    double optimal_duration = this->evaluate(indices, is_valid, f, t, args...);

    std::copy(indices.begin(), indices.end(), optimal_indices.begin());

    this->report_verbose("new best kernel", optimal_duration, optimal_indices,
                         f);

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
        // reset the parameters "below" and start with the first parameter
        // again
        for (size_t i = 0; i < current_index; i++) {
          values[i] = parameters[i].get_value(0);
          indices[i] = 0;
        }
        current_index = 0;

        // evaluate new valid value vector
        if (f.is_verbose()) {
          std::cout << "evaluating combination " << combination_counter
                    << " (out of " << total_combinations << ")" << std::endl;
        }
        combination_counter += 1;
        double duration = this->evaluate(indices, is_valid, f, t, args...);
        if (is_valid && duration < optimal_duration) {
          std::copy(indices.begin(), indices.end(), optimal_indices.begin());
          optimal_duration = duration;
          this->report_verbose("new best kernel", optimal_duration,
                               optimal_indices, f);
        }

      } else {
        // no valid more values, try next parameter "above"
        current_index += 1;
      }
    }
    return optimal_indices;
  }
};
} // namespace tuners
} // namespace autotune
