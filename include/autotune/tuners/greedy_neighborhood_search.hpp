#pragma once

#include "../autotune.hpp"
#include "../parameter.hpp"
#include "abstract_tuner.hpp"
#include "countable_set.hpp"

namespace autotune {
namespace tuners {

template <typename R, typename... Args>
class greedy_neighborhood_search
    : public abstract_tuner<countable_set, R, Args...> {
private:
  size_t iterations;

public:
  greedy_neighborhood_search(
      autotune::cppjit_kernel<R, cppjit::detail::pack<Args...>> &f,
      countable_set &parameters, size_t iterations)
      : abstract_tuner<countable_set, R, Args...>(f, parameters),
        iterations(iterations) {}

private:
  void tune_impl(Args &... args) override {

    std::default_random_engine generator;
    std::bernoulli_distribution distribution_dir(0.5);
    std::uniform_int_distribution<size_t> distribution_para(
        0, this->parameters.size() - 1);

    this->evaluate(args...);

    for (size_t i = 0; i < iterations; i++) {
      if (this->verbose) {
        std::cout << "--------- next iteration: " << i << " ---------"
                  << std::endl;
      }
      countable_set base_parameters = this->optimal_parameters;

      bool is_better = false;

      size_t d = distribution_para(generator);

      if (distribution_dir(generator)) {
        // test next
        this->parameters = base_parameters;
        if (this->parameters[d]->next()) {
          is_better = this->evaluate(args...);
          if (is_better) {
            break;
          }
        }
        // test previous
        this->parameters = base_parameters;
        if (this->parameters[d]->prev()) {
          is_better = this->evaluate(args...);
          if (is_better) {
            break;
          }
        }
      } else {
        // test previous
        this->parameters = base_parameters;
        if (this->parameters[d]->prev()) {
          is_better = this->evaluate(args...);
          if (is_better) {
            break;
          }
        }
        // test next
        this->parameters = base_parameters;
        if (this->parameters[d]->next()) {
          is_better = this->evaluate(args...);
          if (is_better) {
            break;
          }
        }
      }

      this->parameters = this->optimal_parameters;
    }
  }
};
} // namespace tuners
} // namespace autotune
