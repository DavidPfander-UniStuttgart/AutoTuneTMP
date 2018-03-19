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
  size_t changes_per_iteration;

public:
  greedy_neighborhood_search(
      autotune::cppjit_kernel<R, cppjit::detail::pack<Args...>> &f,
      countable_set &parameters, size_t iterations,
      size_t changes_per_iteration)
      : abstract_tuner<countable_set, R, Args...>(f, parameters),
        iterations(iterations), changes_per_iteration(changes_per_iteration) {}

private:
  void tune_impl(Args &... args) override {

    auto gen_dir = detail::make_bernoulli_generator();
    auto gen_changes =
        detail::make_uniform_int_generator(1ul, changes_per_iteration);
    auto gen_para =
        detail::make_uniform_int_generator(0ul, this->parameters.size() - 1ul);

    this->evaluate(args...);

    for (size_t i = 0; i < iterations; i++) {
      if (this->verbose) {
        std::cout << "--------- next iteration: " << i << " ---------"
                  << std::endl;
      }
      countable_set base_parameters = this->optimal_parameters;

      size_t changes = gen_changes();

      for (size_t j = 0; j < changes; j++) {
        size_t par_index = gen_para();

        if (gen_dir()) {
          this->parameters[par_index]->next();
        } else {
          this->parameters[par_index]->prev();
        }
      }
      this->evaluate(args...);
      this->parameters = this->optimal_parameters;
    }
  }
};
} // namespace tuners
} // namespace autotune
