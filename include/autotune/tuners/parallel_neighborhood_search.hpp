#pragma once

#include "abstract_tuner.hpp"
#include "countable_set.hpp"
#include "parameter_result_cache.hpp"

#include <random>

namespace autotune {
namespace tuners {

template <typename R, typename... Args>
class parallel_neighborhood_search
    : public abstract_tuner<countable_set, R, Args...> {
private:
  size_t max_iterations;

public:
  parallel_neighborhood_search(
      autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
      countable_set &parameters, size_t max_iterations)
      : abstract_tuner<countable_set, R, Args...>(f, parameters),
        max_iterations(max_iterations) {}

private:
  virtual void tune_impl(Args &... args) override {
    this->evaluate(args...);

    for (size_t i = 0; i < max_iterations; i++) {
      if (this->verbose) {
        std::cout << "--------- next iteration: " << i << " ---------"
                  << std::endl;
      }
      countable_set base_parameters = this->optimal_parameters;
      std::vector<countable_set> parameters_to_evaluate;
      for (size_t d = 0; d < this->parameters.size(); d++) {
        // test next
        this->parameters = base_parameters;
        if (this->parameters[d]->next()) {
          parameters_to_evaluate.push_back(this->parameters);
        }
        // test previous
        this->parameters = base_parameters;
        if (this->parameters[d]->prev()) {
          parameters_to_evaluate.push_back(this->parameters);
        }
      }
      this->evaluate_parallel(parameters_to_evaluate, args...);
      this->parameters = this->optimal_parameters;
    }
  }

  void reset_impl() override {}
};

} // namespace tuners
} // namespace autotune
