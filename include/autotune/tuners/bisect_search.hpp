#pragma once

#include "../autotune.hpp"
#include "../parameter.hpp"
#include "abstract_tuner.hpp"
#include "limited_set.hpp"

namespace autotune {
namespace tuners {

template <typename R, typename... Args>
class bisect_search : public abstract_tuner<limited_set, R, Args...> {
private:
  size_t iterations;

public:
  bisect_search(autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
              limited_set &parameters, size_t iterations)
      : abstract_tuner<limited_set, R, Args...>(f, parameters),
        iterations(iterations) {}
  bisect_search(autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
              limited_set &parameters, parameter_set allparameters, size_t iterations)
      : abstract_tuner<limited_set, R, Args...>(f, parameters, allparameters),
        iterations(iterations) {}

  limited_set tune(Args &... args) {

    this->result_cache.clear();

    parameter_value_set original_values = this->f.get_parameter_values();

    limited_set optimal_parameters = this->parameters;
    
    std::vector<double> min(this->parameters.size());
    std::vector<double> max(this->parameters.size());
    std::vector<double> mid(this->parameters.size());
    std::vector<std::pair<double, bool>> min_eval(this->parameters.size());
    std::vector<std::pair<double, bool>> max_eval(this->parameters.size());
    std::vector<std::pair<double, bool>> mid_eval(this->parameters.size());
    
    for (size_t p_idx = 0; p_idx < this->parameters.size(); p_idx++) {
      auto &p = this->parameters[p_idx];
      double range = p->get_max() - p->get_min();
      min[p_idx] = p->get_min() + 0.25*range;
      mid[p_idx] = p->get_min() + 0.50*range;
      max[p_idx] = p->get_min() + 0.75*range;
      p->set_value(mid[p_idx]);
      min_eval[p_idx].second = false;
      max_eval[p_idx].second = false;
      mid_eval[p_idx].second = false;
    }

    for (size_t i = 0; i < iterations; i++) {
      for (size_t p_idx = 0; p_idx < this->parameters.size(); p_idx++) {
        auto &p = this->parameters[p_idx];
        if (!min_eval[p_idx].second) {
          p->set_value(min[p_idx]);
          min_eval[p_idx].first = this->evaluate(min_eval[p_idx].second, args...);
        }
        if (!max_eval[p_idx].second) {
          p->set_value(max[p_idx]);
          max_eval[p_idx].first = this->evaluate(max_eval[p_idx].second, args...);
        }
        if (!mid_eval[p_idx].second) {
          p->set_value(mid[p_idx]);
          mid_eval[p_idx].first = this->evaluate(mid_eval[p_idx].second, args...);
        }
        double opt_val = min[p_idx];
        std::pair<double, bool> opt = min_eval[p_idx];
        if (!opt.second || (mid_eval[p_idx].second && mid_eval[p_idx].first < opt.first)) {
          opt = mid_eval[p_idx];
          opt_val = mid[p_idx];
        }
        if (!opt.second || (max_eval[p_idx].second && max_eval[p_idx].first < opt.first)) {
          opt = max_eval[p_idx];
          opt_val = max[p_idx];
        }
        if (opt.second) {
          mid[p_idx] = opt_val;
          mid_eval[p_idx] = opt;
          double range = max[p_idx] - min[p_idx];
          min[p_idx] = opt_val - 0.25*range;
          max[p_idx] = opt_val + 0.25*range;
          min_eval[p_idx].second = false;
          max_eval[p_idx].second = false;
          optimal_parameters = this->parameters;
        } else {
          min[p_idx] = 0.5*(min[p_idx] + p->get_min());
          max[p_idx] = 0.5*(max[p_idx] + p->get_max());
        }
        p->set_value(mid[p_idx]);
      }
    }

    this->f.set_parameter_values(original_values);
    return optimal_parameters;
  }
};
} // namespace tuners
} // namespace autotune
