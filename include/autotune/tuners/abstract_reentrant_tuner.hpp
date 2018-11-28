#pragma once

#include "../abstract_kernel.hpp"
#include "apply_parameters.hpp"
#include "csv_reporter.hpp"
#include "evaluate.hpp"
#include "parameter_result_cache.hpp"
#include "with_tests.hpp"
#include <chrono>

namespace autotune {

template <typename parameter_interface, typename R, typename... Args>
class abstract_reentrant_tuner
    : public std::conditional<!std::is_same<R, void>::value,
                              with_tests<R, Args...>,
                              without_tests<R, Args...>>::type {
protected:
  autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f;
  // always-unadjusted, state managed by tuner impl.!
  parameter_interface parameters;
  parameter_interface optimal_parameters; // adjusted
  double optimal_duration;
  bool verbose;

  parameter_result_cache result_cache;

  std::function<void(parameter_interface &)> parameter_adjustment_functor;
  std::function<void(parameter_value_set &)>
      parameter_values_adjustment_functor;

  std::function<double(Args...)> weigh_input_functor;

  double weight;

  size_t repetitions;

  uint64_t evaluations;
  // excluding skipped or failed evaluations
  uint64_t evaluations_passed;

  virtual void tune_impl(Args &... args) = 0;

public:
  std::shared_ptr<csv_reporter> reporter;

  abstract_reentrant_tuner(
      autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
      parameter_interface &parameters)
      : f(f), parameters(parameters), optimal_duration(-1.0), verbose(false),
        weight(1.0), repetitions(1), evaluations(0), evaluations_passed(0) {}

  void reentrant_tune(Args &... args) { tune_impl(args...); }

  evaluate_t evaluate(Args &... args) {
    evaluations += 1;
    parameter_value_set original_kernel_values = f.get_parameter_values();
    bool do_evaluate =
        apply_parameters(f, parameters, parameter_adjustment_functor,
                         parameter_values_adjustment_functor, result_cache,
                         verbose); // does adjustment if applicable
    if (do_evaluate) {
      if (weigh_input_functor) {
        weight = weigh_input_functor(args...);
        if (verbose) {
          std::cout << "info: weight: " << weight << std::endl;
        }
      }
      evaluate_t is_better =
          detail::evaluate_parameters<abstract_reentrant_tuner,
                                      parameter_interface, R, Args...>(
              *this, f, parameters, args...);
      f.set_parameter_values(original_kernel_values);
      if (is_better != evaluate_t::skipped_failed) {
        evaluations_passed += 1;
      }
      return is_better;
    }
    return evaluate_t::skipped_failed;
  }

  void set_verbose(bool verbose) { this->verbose = verbose; }

  void set_write_measurement(const std::string &scenario_name) {
    reporter = std::make_shared<csv_reporter>(scenario_name,
                                              to_parameter_values(parameters));
  }

  void set_parameter_adjustment_functor(
      std::function<void(parameter_interface &)> parameter_adjustment_functor) {
    this->parameter_adjustment_functor = parameter_adjustment_functor;
    this->parameter_values_adjustment_functor = nullptr;
  }

  void set_parameter_values_adjustment_functor(
      std::function<void(parameter_value_set &)>
          parameter_values_adjustment_functor) {
    this->parameter_values_adjustment_functor =
        parameter_values_adjustment_functor;
    this->parameter_adjustment_functor = nullptr;
  }

  void
  set_weigh_input_functor(std::function<double(Args...)> weigh_input_functor) {
    this->weigh_input_functor = weigh_input_functor;
  }

  // execute kernel multiple times to average across the result
  void set_repetitions(size_t repetitions) { this->repetitions = repetitions; }

  parameter_interface &get_parameters() { return parameters; }

  parameter_interface &get_optimal_parameters() { return optimal_parameters; }

  evaluate_t update_parameters(parameter_interface &adjusted_candidate,
                               double candidate_duration,
                               double duration_compile) {
    double weighted_candidate_duration = weight * candidate_duration;
    if (reporter) {
      reporter->write_measurement(to_parameter_values(adjusted_candidate),
                                  weighted_candidate_duration,
                                  candidate_duration, duration_compile);
    }

    evaluate_t is_better = evaluate_t::worse_equal;
    if (optimal_duration < 0.0 ||
        weighted_candidate_duration < optimal_duration) {
      optimal_duration = weighted_candidate_duration;
      optimal_parameters = adjusted_candidate;
      if (verbose) {
        std::cout << "new best kernel; duration: "
                  << weighted_candidate_duration << std::endl;
        parameters.print_values();
      }
      is_better = evaluate_t::better;
    }
    return is_better;
  }

  parameter_result_cache &get_result_cache() { return result_cache; }

  bool is_verbose() { return verbose; }

  size_t get_repetitions() { return repetitions; }

  virtual void reset_impl() = 0;

  void reset() {
    result_cache.clear();
    optimal_duration = -1.0;
    optimal_parameters = parameters;
    evaluations = 0;
    evaluations_passed = 0;
    reset_impl();
  }

  int64_t get_evaluations() { return evaluations; }

  int64_t get_passed_evaluations() { return evaluations_passed; }

  double get_current_input_weight() { return weight; }
};

} // namespace autotune
