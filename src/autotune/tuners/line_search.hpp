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
  std::function<bool(const R &)> &t;
  size_t max_iterations;
  size_t restarts;
  std::vector<size_t> initial_indices_guess;

public:
  line_search(autotune::kernel<R, cppjit::detail::pack<Args...>> &f,
              std::function<bool(const R &)> &t, size_t max_iterations,
              size_t restarts, std::vector<size_t> &initial_indices_guess)
      : f(f), t(t), max_iterations(max_iterations), restarts(restarts),
        initial_indices_guess(initial_indices_guess) {}

  std::vector<size_t> tune(Args &... args) {

    f.write_header();
    std::vector<tunable_parameter> &parameters = f.get_parameters();

    if (initial_indices_guess.size() != parameters.size()) {
      throw;
    }

    // evaluate initial guess
    bool is_valid = true;
    std::vector<size_t> optimal_indices = initial_indices_guess;
    double optimal_duration =
        this->evaluate(optimal_indices, is_valid, f, t, args...);

    size_t counter = 0;
    size_t cur_index = 0;
    while (counter < max_iterations) {
      if (f.is_verbose()) {
        std::cout << "cur_index:" << cur_index << std::endl;
        std::cout << "current best indices:" << std::endl;
        f.print_values(optimal_indices);
      }

      for (size_t inner_index = 0; inner_index < parameters[cur_index].size();
           inner_index++) {

        std::vector<size_t> indices_attempt(optimal_indices);
        indices_attempt[cur_index] = inner_index;

        if (f.is_verbose()) {
          std::cout << "current attempt:" << std::endl;
          f.print_values(indices_attempt);
        }

        // if a valid new index value was found, test it

        double duration =
            this->evaluate(indices_attempt, is_valid, f, t, args...);
        if (is_valid && duration < optimal_duration) {
          optimal_duration = duration;
          optimal_indices = indices_attempt;
          if (f.is_verbose()) {
            this->report_verbose("new best kernel", optimal_duration,
                                 optimal_indices, f);
          }
        }
      }
      cur_index = (cur_index + 1) % parameters.size();
      counter += 1;
    }

    return optimal_indices;
  }
};

} // namespace tuners
} // namespace autotune
