#pragma once

#include "../parameter.hpp"
#include "common.hpp"

#include <random>

namespace autotune {
namespace tuners {

template <class... Args> class line_search;

template <typename R, typename... Args>
class line_search<autotune::kernel<R, cppjit::detail::pack<Args...>>>
    : public abstract_tuner<R, Args...> {
private:
  autotune::kernel<R, cppjit::detail::pack<Args...>> &f;
  size_t max_iterations;
  size_t restarts;
  // std::vector<size_t> initial_indices_guess;

public:
  line_search(autotune::kernel<R, cppjit::detail::pack<Args...>> &f,
              size_t max_iterations, size_t restarts
              // , std::vector<size_t> &initial_indices_guess
              )
      : abstract_tuner<R, Args...>(), f(f), max_iterations(max_iterations),
        restarts(restarts) //, initial_indices_guess(initial_indices_guess)
  {}

  // line_search(autotune::kernel<R, cppjit::detail::pack<Args...>> &f,
  //             std::function<bool(const R &)> &t, size_t max_iterations,
  //             size_t restarts, std::vector<size_t> &initial_indices_guess)
  //     : abstract_tuner<R, Args...>(t), f(f), max_iterations(max_iterations),
  //       restarts(restarts), initial_indices_guess(initial_indices_guess) {}

  parameter_set tune(Args &... args) {

    f.write_header();
    parameter_set &parameters = f.get_parameters();
    // set to initial guess (or whatever the parameter type is doing)
    for (size_t i = 0; i < parameters.size(); i++) {
      parameters[i]->reset();
    }

    // if (initial_indices_guess.size() != parameters.size()) {
    //   throw;
    // }

    // memorize original parameters
    parameter_set original_parameters = parameters.clone();

    // evaluate initial guess
    bool is_valid = true;
    parameter_set optimal_parameters = parameters.clone();

    double optimal_duration = this->evaluate(is_valid, f, args...);

    size_t counter = 0;
    size_t cur_index = 0;
    while (counter < max_iterations) {
      if (f.is_verbose()) {
        std::cout << "cur_index:" << cur_index << std::endl;
        std::cout << "current best indices:" << std::endl;
        f.print_values();
      }

      for (size_t inner_index = 0;
           inner_index < parameters[cur_index]->count_values(); inner_index++) {

        // std::vector<size_t> indices_attempt(optimal_indices);
        // indices_attempt[cur_index] = inner_index;
        auto p = std::dynamic_pointer_cast<fixed_set_parameter>(
            parameters[cur_index]);
        p->set_index(inner_index);

        if (f.is_verbose()) {
          std::cout << "current attempt:" << std::endl;
          f.print_values();
        }

        // if a valid new index value was found, test it

        double duration = this->evaluate(is_valid, f, args...);
        if (is_valid && duration < optimal_duration) {
          optimal_duration = duration;
          optimal_parameters = f.get_parameters();
          if (f.is_verbose()) {
            this->report_verbose("new best kernel", optimal_duration, f);
          }
        }
      }
      cur_index = (cur_index + 1) % parameters.size();
      counter += 1;
    }

    f.replace_parameters(original_parameters);

    return optimal_parameters;
  }
};

} // namespace tuners
} // namespace autotune
