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

public:
  monte_carlo(autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
              randomizable_set &parameters, size_t iterations)
      : abstract_tuner<randomizable_set, R, Args...>(f, parameters),
        iterations(iterations) {}

private:
  void tune_impl(Args &... args) override {

    // this->result_cache.clear();

    // parameter_value_set original_values = this->f.get_parameter_values();

    // bool first = true;
    // bool is_valid = true;
    // double optimal_duration = this->evaluate(is_valid, args...);
    // randomizable_set optimal_parameters = this->parameters;

    // if (is_valid) {
    //   first = false;
    //   std::cout << "initial values tested" << std::endl;
    // }

    for (size_t i = 0; i < iterations; i++) {

      for (size_t parameter_index = 0;
           parameter_index < this->parameters.size(); parameter_index++) {
        auto &p = this->parameters[parameter_index];
        p->set_random_value();
      }

      this->evaluate(args...);
      // if (is_valid && (first || duration < optimal_duration)) {
      //   first = false;
      //   optimal_parameters = this->parameters;
      //   optimal_duration = duration;
      //   this->report_verbose("new best kernel", optimal_duration,
      //                        optimal_parameters);
      // }
    }

    // this->f.set_parameter_values(original_values);
    // if (this->parameter_adjustment_functor) {
    //   this->parameter_adjustment_functor(optimal_parameters);
    // }
    // return optimal_parameters;
  }
};
} // namespace tuners
} // namespace autotune
