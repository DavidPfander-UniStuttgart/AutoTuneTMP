#pragma once

#include "../abstract_kernel.hpp"
#include "../parameter.hpp"
#include "abstract_tuner.hpp"
#include "countable_set.hpp"

namespace autotune {
namespace tuners {

template <typename R, typename... Args>
class bruteforce : public abstract_tuner<countable_set, R, Args...> {
public:
  bruteforce(autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
             countable_set &parameters)
      : abstract_tuner<countable_set, R, Args...>(f, parameters) {}

private:
  void tune_impl(Args &... args) override {

    size_t total_combinations = 1.0;
    for (size_t i = 0; i < this->parameters.size(); i++) {
      total_combinations *= this->parameters[i]->count_values();
    }

    if (this->verbose) {
      std::cout << "total combinations to test: " << total_combinations
                << std::endl;
    }

    // brute-force tuner
    for (size_t i = 0; i < this->parameters.size(); i++) {
      this->parameters[i]->set_min();
    }

    // evaluate initial vector, always valid
    size_t combination_counter = 1;
    if (this->verbose) {
      std::cout << "evaluating combination " << combination_counter
                << " (out of " << total_combinations << ")" << std::endl;
    }
    combination_counter += 1;
    this->evaluate(args...);

    size_t current_index = 0;
    while (true) {
      // left the range of valid indices, done!
      if (current_index == this->parameters.size()) {
        break;
      }

      // the is another value for the current parameter
      if (this->parameters[current_index]->next()) {
        // reset the parameters "below" and start with the first parameter
        // again
        for (size_t i = 0; i < current_index; i++) {
          this->parameters[i]->set_min();
        }
        current_index = 0;

        // evaluate new valid value vector
        if (this->verbose) {
          std::cout << "evaluating combination " << combination_counter
                    << " (out of " << total_combinations << ")" << std::endl;
        }
        combination_counter += 1;
        this->evaluate(args...);

      } else {
        // no valid more values, try next parameter "above"
        current_index += 1;
      }
    }
  }
};
} // namespace tuners
} // namespace autotune
