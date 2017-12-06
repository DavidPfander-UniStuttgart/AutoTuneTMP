#pragma once

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
  bool verbose;

public:
  line_search(autotune::kernel<R, cppjit::detail::pack<Args...>> &f,
              size_t max_iterations, size_t restarts
              // , std::vector<size_t> &initial_indices_guess
              )
      : abstract_tuner<R, Args...>(), f(f), max_iterations(max_iterations),
        restarts(restarts),
        verbose(false) //, initial_indices_guess(initial_indices_guess)
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

    // if (f.is_verbose()) {
    //   std::cout << "evaluating initial parameter combination" << std::endl;
    //   std::cout << "initial values:" << std::endl;
    //   f.print_values();
    // }
    // double optimal_duration = this->evaluate(is_valid, f, args...);
    double optimal_duration;
    bool first = true;

    size_t counter = 0;
    size_t cur_index = 0;
    while (counter < max_iterations) {
      if (verbose) {
        std::cout << "current parameter index: " << cur_index << std::endl;
      }

      auto p = std::dynamic_pointer_cast<step_parameter>(parameters[cur_index]);
      // p->set_index(0);
      p->reset();
      while (true) {

        if (verbose) {
          std::cout << "current attempt:" << std::endl;
          f.print_values();
        }

        // if a valid new index value was found, test it
        double duration = this->evaluate(is_valid, f, args...);
        if (is_valid && first && duration < optimal_duration) {
          first = false;
          optimal_parameters = f.get_parameters().clone();
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
      p->reset();
      // do not evaluate resetted value
      while (p->prev()) {

        if (verbose) {
          std::cout << "current attempt:" << std::endl;
          f.print_values();
        }

        // if a valid new index value was found, test it
        double duration = this->evaluate(is_valid, f, args...);
        if (is_valid && (first || duration < optimal_duration)) {
          first = false;
          optimal_parameters = f.get_parameters().clone();
          optimal_duration = duration;
          if (verbose) {
            this->report_verbose("new best kernel", optimal_duration, f);
          }
        }
      }
      cur_index = (cur_index + 1) % parameters.size();
      counter += 1;
    }

    f.set_parameters(original_parameters);

    return optimal_parameters;
  }

  void set_verbose(bool verbose) { this->verbose = verbose; }
};

} // namespace tuners
} // namespace autotune
