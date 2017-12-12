#pragma once

#include "common.hpp"
#include "countable_set.hpp"

#include <random>

namespace autotune {
namespace tuners {

template <class... Args> class line_search;

template <typename R, typename... Args>
class line_search<autotune::cppjit_kernel<R, cppjit::detail::pack<Args...>>>
    : public abstract_tuner<countable_set, R, Args...> {
private:
  autotune::cppjit_kernel<R, cppjit::detail::pack<Args...>> &f;
  size_t max_iterations;
  size_t restarts;
  countable_set &parameters;

public:
  line_search(autotune::cppjit_kernel<R, cppjit::detail::pack<Args...>> &f,
              size_t max_iterations, size_t restarts, countable_set &parameters)
      : abstract_tuner<countable_set, R, Args...>(), f(f),
        max_iterations(max_iterations), restarts(restarts),
        parameters(parameters) {}

  countable_set tune(Args &... args) {

    f.write_header();

    // set to initial guess (or whatever the parameter type is doing)
    for (size_t i = 0; i < parameters.size(); i++) {
      parameters[i]->set_min();
    }

    // memorize original parameters
    parameter_value_set original_values = f.get_parameter_values();

    // evaluate initial guess
    bool is_valid = true;
    countable_set optimal_parameters = parameters.clone();

    double optimal_duration;
    bool first = true;

    size_t counter = 0;
    size_t cur_index = 0;
    while (counter < max_iterations) {
      if (this->verbose) {
        std::cout << "current parameter index: " << cur_index << std::endl;
      }

      auto &p = parameters[cur_index];
      // p->set_index(0);
      p->set_initial();
      while (true) {

        // if a valid new index value was found, test it
        double duration = this->evaluate(is_valid, parameters, f, args...);
        if (is_valid && (first || duration < optimal_duration)) {
          first = false;
          optimal_parameters = parameters.clone();
          optimal_duration = duration;
          if (this->verbose) {
            this->report_verbose("new best kernel", optimal_duration,
                                 parameters);
          }
        }
        if (!p->next()) {
          break;
        }
      }

      p->set_initial();
      // do not evaluate resetted value
      while (p->prev()) {

        // if a valid new index value was found, test it
        double duration = this->evaluate(is_valid, parameters, f, args...);
        if (is_valid && (first || duration < optimal_duration)) {
          first = false;
          optimal_parameters = parameters.clone();
          optimal_duration = duration;
          if (this->verbose) {
            this->report_verbose("new best kernel", optimal_duration,
                                 parameters);
          }
        }
      }
      cur_index = (cur_index + 1) % parameters.size();
      counter += 1;
      parameters = optimal_parameters;
    }

    f.set_parameter_values(original_values);

    return optimal_parameters;
  }
};

} // namespace tuners
} // namespace autotune
