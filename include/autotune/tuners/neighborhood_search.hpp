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
      autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
      countable_set &parameters, size_t iterations)
      : abstract_tuner<countable_set, R, Args...>(f, parameters),
        iterations(iterations) {}

private:
  void tune_impl(Args &... args) override {
    this->evaluate(args...);

    for (size_t i = 0; i < iterations; i++) {
      if (this->verbose) {
        std::cout << "--------- next iteration: " << i << " ---------"
                  << std::endl;
      }
      countable_set base_parameters = this->optimal_parameters;

      for (size_t d = 0; d < this->parameters.size(); d++) {
        // test next
        this->parameters = base_parameters;
        if (this->parameters[d]->next()) {
          this->evaluate(args...);
        }
        // test previous
        this->parameters = base_parameters;
        if (this->parameters[d]->prev()) {
          this->evaluate(args...);
        }
      }
      this->parameters = this->optimal_parameters;
    }
  }

  void reset_impl() override {}
};
} // namespace tuners
} // namespace autotune
