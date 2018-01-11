#pragma once

#include "../autotune.hpp"
#include "../parameter.hpp"
#include "abstract_tuner.hpp"
#include "countable_set.hpp"

namespace autotune {
namespace tuners {

template <typename R, typename... Args>
class neighborhood_search : public abstract_tuner<countable_set, R, Args...> {
private:
  size_t iterations;

public:
  neighborhood_search(
      autotune::cppjit_kernel<R, cppjit::detail::pack<Args...>> &f,
      countable_set &parameters, size_t iterations)
      : abstract_tuner<countable_set, R, Args...>(f, parameters),
        iterations(iterations) {}

  countable_set tune(Args &... args) {

    this->result_cache.clear();

    parameter_value_set original_values = this->f.get_parameter_values();

    bool is_valid = true;
    double optimal_duration = -1.0;
    double duration = this->evaluate(is_valid, args...);
    countable_set optimal_parameters;
    if (is_valid) {
      optimal_parameters = this->parameters;
      optimal_duration = duration;
    }

    for (size_t i = 0; i < iterations; i++) {
      if (this->verbose) {
        std::cout << "--------- next iteration: " << i << " ---------"
                  << std::endl;
      }
      countable_set base_parameters = optimal_parameters;

      for (size_t d = 0; d < this->parameters.size(); d++) {
        // test next
        this->parameters = base_parameters;
        if (this->parameters[d]->next()) {
          duration = this->evaluate(is_valid, args...);
          if (is_valid &&
              (optimal_duration == -1.0 || duration < optimal_duration)) {
            optimal_parameters = this->parameters;
            optimal_duration = duration;
            this->report_verbose("new best kernel", optimal_duration,
                                 this->parameters);
          }
        }
        // test previous
        this->parameters = base_parameters;
        if (this->parameters[d]->prev()) {
          duration = this->evaluate(is_valid, args...);
          if (is_valid &&
              (optimal_duration == -1.0 || duration < optimal_duration)) {
            optimal_parameters = this->parameters;
            optimal_duration = duration;
            this->report_verbose("new best kernel", optimal_duration,
                                 this->parameters);
          }
        }
      }
      this->parameters = optimal_parameters;
    }

    this->f.set_parameter_values(original_values);
    return optimal_parameters;
  }
};
} // namespace tuners
} // namespace autotune
