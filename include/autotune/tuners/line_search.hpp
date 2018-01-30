#pragma once

#include "abstract_tuner.hpp"
#include "countable_set.hpp"
#include "parameter_result_cache.hpp"

#include <random>

namespace autotune {
namespace tuners {

template <typename R, typename... Args>
class line_search : public abstract_tuner<countable_set, R, Args...> {
private:
  size_t max_iterations;
  size_t restarts;

  // void evaluate_parameter_set(countable_set &optimal_parameters,
  //                             double &optimal_duration, Args &... args) {
  //   // if a valid new index value was found, test it
  //   bool did_eval = true;
  //   double duration = this->evaluate(did_eval, args...);
  //   if (did_eval && (optimal_duration == -1.0 || duration <
  //   optimal_duration)) {
  //     optimal_parameters = this->parameters;
  //     optimal_duration = duration;
  //     if (this->verbose) {
  //       this->report_verbose("new best kernel", optimal_duration,
  //                            this->parameters);
  //     }
  //   }
  // }

public:
  line_search(autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
              countable_set &parameters, size_t max_iterations,
              size_t restarts = 0)
      : abstract_tuner<countable_set, R, Args...>(f, parameters),
        max_iterations(max_iterations), restarts(restarts) {}

private:
  virtual void tune_impl(Args &... args) override {

    // // set to initial guess (or whatever the parameter type is doing)
    // // for (size_t i = 0; i < this->parameters.size(); i++) {
    // //  this->parameters[i]->set_min();
    // //}

    // // evaluate initial guess
    // countable_set optimal_parameters = this->parameters;

    // double optimal_duration = -1.0;

    size_t counter = 0;
    size_t cur_index = 0;
    while (counter < max_iterations) {
      if (this->verbose) {
        std::cout << "current parameter index: " << cur_index << std::endl;
      }

      auto &p = this->parameters[cur_index];
      std::string old_value = p->get_value();
      p->set_initial();

      // evaluate_parameter_set(optimal_parameters, optimal_duration, args...);
      this->evaluate(args...);

      while (p->next()) {
        // evaluate_parameter_set(optimal_parameters, optimal_duration,
        // args...);
        this->evaluate(args...);
      }

      p->set_initial();
      // do not evaluate resetted value
      while (p->prev()) {
        // evaluate_parameter_set(optimal_parameters, optimal_duration,
        // args...);
        this->evaluate(args...);
      }
      cur_index = (cur_index + 1) % this->parameters.size();
      counter += 1;
      this->parameters = this->optimal_parameters;
    }

    // return optimal_parameters;
  }
};

} // namespace tuners
} // namespace autotune
