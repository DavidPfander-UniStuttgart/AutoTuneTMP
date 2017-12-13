#pragma once

#include "../autotune.hpp"
#include "../parameter.hpp"
#include "common.hpp"
#include "limited_set.hpp"

#include <random>

namespace autotune {
namespace tuners {

template <typename R, typename... Args>
class monte_carlo : public abstract_tuner<limited_set, R, Args...> {
private:
  autotune::cppjit_kernel<R, cppjit::detail::pack<Args...>> &f;
  limited_set &parameters;
  size_t iterations;

public:
  monte_carlo(autotune::cppjit_kernel<R, cppjit::detail::pack<Args...>> &f,
              limited_set &parameters, size_t iterations)
      : f(f), parameters(parameters), iterations(iterations) {}

  limited_set tune(Args &... args) {

    parameter_value_set original_values = f.get_parameter_values();

    bool first = true;
    bool is_valid = true;
    limited_set optimal_parameters = parameters.clone();
    double optimal_duration =
        this->evaluate(is_valid, optimal_parameters, f, args...);

    if (is_valid) {
      first = false;
      std::cout << "initial values tested" << std::endl;
    }

    for (size_t i = 0; i < iterations; i++) {

      limited_set current_parameters = optimal_parameters.clone();

      for (size_t parameter_index = 0;
           parameter_index < current_parameters.size(); parameter_index++) {
        auto &p = current_parameters[parameter_index];

        if (p->is_integer_parameter()) {
          std::uniform_int_distribution<size_t> distribution(
              static_cast<size_t>(p->get_min()),
              static_cast<size_t>(p->get_max()));
          std::random_device rd;
          std::default_random_engine generator(rd());
          p->set_value(static_cast<double>(distribution(generator)));
        } else {
          std::uniform_real_distribution<double> distribution(p->get_min(),
                                                              p->get_max());
          std::random_device rd;
          std::default_random_engine generator(rd());
          p->set_value(distribution(generator));
        }
      }

      double duration =
          this->evaluate(is_valid, current_parameters, f, args...);
      if (is_valid && (first || duration < optimal_duration)) {
        first = false;
        optimal_parameters = current_parameters.clone();
        optimal_duration = duration;
        this->report_verbose("new best kernel", optimal_duration,
                             optimal_parameters);
      }
    }

    f.set_parameter_values(original_values);
    return optimal_parameters;
  }
};
} // namespace tuners
} // namespace autotune
