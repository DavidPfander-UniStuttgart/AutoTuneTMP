#pragma once

#include "../autotune.hpp"
#include "../parameter.hpp"
#include "abstract_tuner.hpp"
#include "randomizable_set.hpp"

namespace autotune {
namespace tuners {

template <typename R, typename... Args>
class monte_carlo : public abstract_tuner<randomizable_set, R, Args...> {
private:
  size_t iterations;
  size_t max_attempts;

public:
  monte_carlo(autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
              randomizable_set &parameters, size_t iterations,
              size_t max_attempts)
      : abstract_tuner<randomizable_set, R, Args...>(f, parameters),
        iterations(iterations), max_attempts(max_attempts) {}

private:
  void tune_impl(Args &... args) override {
    size_t i = 0;
    size_t attempts = 0;
    while (i < iterations && attempts < max_attempts) {
      for (size_t parameter_index = 0;
           parameter_index < this->parameters.size(); parameter_index += 1) {
        auto &p = this->parameters[parameter_index];
        p->set_random_value();
      }

      evaluate_t state = this->evaluate(args...);
      if (state != evaluate_t::skipped_failed) {
        i += 1;
      }
      attempts += 1;
    }
  }

  void reset_impl() override {}
};
} // namespace tuners
} // namespace autotune
