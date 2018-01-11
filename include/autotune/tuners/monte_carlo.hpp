#pragma once

#include "../autotune.hpp"
#include "../parameter.hpp"
#include "abstract_tuner.hpp"
#include "limited_set.hpp"

#include <random>

namespace autotune {
namespace tuners {

template <typename R, typename... Args>
class monte_carlo : public abstract_tuner<limited_set, R, Args...> {
private:
  size_t iterations;

public:
  monte_carlo(autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
              limited_set &parameters, size_t iterations)
      : abstract_tuner<limited_set, R, Args...>(f, parameters),
        iterations(iterations) {}

  limited_set tune(Args &... args) {

    this->result_cache.clear();

    parameter_value_set original_values = this->f.get_parameter_values();

    bool first = true;
    bool is_valid = true;
    double optimal_duration = this->evaluate(is_valid, args...);
    limited_set optimal_parameters = this->parameters;

    if (is_valid) {
      first = false;
      std::cout << "initial values tested" << std::endl;
    }

    for (size_t i = 0; i < iterations; i++) {

      for (size_t parameter_index = 0;
           parameter_index < this->parameters.size(); parameter_index++) {
        auto &p = this->parameters[parameter_index];

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

      double duration = this->evaluate(is_valid, args...);
      if (is_valid && (first || duration < optimal_duration)) {
        first = false;
        optimal_parameters = this->parameters;
        optimal_duration = duration;
        this->report_verbose("new best kernel", optimal_duration,
                             optimal_parameters);
      }
    }

    this->f.set_parameter_values(original_values);
    return optimal_parameters;
  }
};
} // namespace tuners
} // namespace autotune
