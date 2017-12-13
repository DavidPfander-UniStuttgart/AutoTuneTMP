#pragma once

#include "../autotune.hpp"
#include "../parameter.hpp"
#include "common.hpp"
#include "countable_set.hpp"

namespace autotune {
namespace tuners {

template <typename R, typename... Args>
class neighborhood_search : public abstract_tuner<countable_set, R, Args...> {
private:
  autotune::cppjit_kernel<R, cppjit::detail::pack<Args...>> &f;
  countable_set &parameters;
  size_t iterations;

public:
  neighborhood_search(
      autotune::cppjit_kernel<R, cppjit::detail::pack<Args...>> &f,
      countable_set &parameters, size_t iterations)
      : f(f), parameters(parameters), iterations(iterations) {}

  countable_set tune(Args &... args) {

    parameter_value_set original_values = f.get_parameter_values();

    bool first = true;
    bool is_valid = true;
    countable_set optimal_parameters = parameters.clone();
    double optimal_duration =
        this->evaluate(is_valid, optimal_parameters, f, args...);

    if (is_valid) {
      first = false;
    }

    for (size_t i = 0; i < iterations; i++) {
      if (this->verbose) {
        std::cout << "--------- next iteration: " << i << " ---------"
                  << std::endl;
      }
      countable_set base_parameters = optimal_parameters.clone();
      for (size_t d = 0; d < parameters.size(); d++) {
        // test next
        countable_set current_parameters = base_parameters.clone();
        if (current_parameters[d]->next()) {
          double duration =
              this->evaluate(is_valid, current_parameters, f, args...);
          if (is_valid && (first || duration < optimal_duration)) {
            first = false;
            optimal_parameters = current_parameters.clone();
            optimal_duration = duration;
            this->report_verbose("new best kernel", optimal_duration,
                                 parameters);
          }
        }
        // test previous
        current_parameters = base_parameters.clone();
        if (current_parameters[d]->prev()) {
          double duration =
              this->evaluate(is_valid, current_parameters, f, args...);
          if (is_valid && (first || duration < optimal_duration)) {
            first = false;
            optimal_parameters = current_parameters.clone();
            optimal_duration = duration;
            this->report_verbose("new best kernel", optimal_duration,
                                 parameters);
          }
        }
      }
    }

    f.set_parameter_values(original_values);
    return optimal_parameters;
  }
};
} // namespace tuners
} // namespace autotune
