#pragma once

#include "common.hpp"
#include "countable_set.hpp"

#include <random>

namespace autotune {
namespace tuners {

template <class... Args> class line_search;

template <typename R, typename... Args>
class line_search<autotune::kernel<R, cppjit::detail::pack<Args...>>>
    : public abstract_tuner<countable_set, R, Args...> {
private:
  autotune::kernel<R, cppjit::detail::pack<Args...>> &f;
  size_t max_iterations;
  size_t restarts;
  // std::vector<size_t> initial_indices_guess;
  bool verbose;
  countable_set &parameters;

public:
  line_search(autotune::kernel<R, cppjit::detail::pack<Args...>> &f,
              size_t max_iterations, size_t restarts, countable_set &parameters)
      : abstract_tuner<countable_set, R, Args...>(), f(f), max_iterations(max_iterations),
        restarts(restarts), verbose(false),
        parameters(parameters) //, initial_indices_guess(initial_indices_guess)
  {}

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
      if (verbose) {
        std::cout << "current parameter index: " << cur_index << std::endl;
      }

      auto &p = parameters[cur_index];
      // p->set_index(0);
      p->set_initial();
      while (true) {

        if (verbose) {
          std::cout << "current attempt:" << std::endl;
          f.print_values();
        }

        // if a valid new index value was found, test it
        double duration = this->evaluate(is_valid, parameters, f, args...);
        if (is_valid && (first || duration < optimal_duration)) {
          first = false;
          optimal_parameters = parameters.clone();
          optimal_duration = duration;
          if (verbose) {
            this->report_verbose("new best kernel", optimal_duration, f);
          }
        }
        if (!p->next()) {
          break;
        }
      }

      // in case reset does not lead to index set to zero
      p->set_initial();
      // do not evaluate resetted value
      while (p->prev()) {

        if (verbose) {
          std::cout << "current attempt:" << std::endl;
          f.print_values();
        }

        // if a valid new index value was found, test it
        double duration = this->evaluate(is_valid, parameters, f, args...);
        if (is_valid && (first || duration < optimal_duration)) {
          first = false;
          optimal_parameters = parameters.clone();
          optimal_duration = duration;
          if (verbose) {
            this->report_verbose("new best kernel", optimal_duration, f);
          }
        }
      }
      cur_index = (cur_index + 1) % parameters.size();
      counter += 1;
    }

    f.set_parameter_values(original_values);

    return optimal_parameters;
  }

  void set_verbose(bool verbose) { this->verbose = verbose; }
};

} // namespace tuners
} // namespace autotune
