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
class abstract_tuner
    : public std::conditional<!std::is_same<R, void>::value,
                              with_tests<R, Args...>,
                              without_tests<R, Args...>>::type {
protected:
  autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f;
  // always-unadjusted, state managed by tuner impl.!
  parameter_interface &parameters;
  parameter_interface optimal_parameters; // adjusted
  double optimal_duration;
  bool verbose;

  parameter_result_cache result_cache;

  std::function<void(parameter_interface &)> parameter_adjustment_functor;
  std::function<void(parameter_value_set &)>
      parameter_values_adjustment_functor;

  size_t repetitions;

  bool clear_tuner;

  uint64_t evaluations;
  // excluding skipped or failed evaluations
  uint64_t evaluations_passed;

  bool write_measurement;
  std::string scenario_name;
  uint64_t tune_counter;

  virtual void tune_impl(Args &... args) = 0;

public:
  std::shared_ptr<csv_reporter> reporter;

  abstract_tuner(autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
                 parameter_interface &parameters)
      : f(f), parameters(parameters), optimal_duration(-1.0), verbose(false),
        repetitions(1), clear_tuner(true), evaluations(0),
        evaluations_passed(0), write_measurement(false), tune_counter(0) {}

  parameter_interface tune(Args &... args) {
    // if first run or auto clear is active
    if (clear_tuner || optimal_duration < 0) {
      result_cache.clear();
      optimal_duration = -1.0;
      optimal_parameters = parameters;
      evaluations = 0;
      evaluations_passed = 0;
      tune_counter += 1;
      if (write_measurement) {
        reporter = std::make_shared<csv_reporter>(
            scenario_name, to_parameter_values(parameters), tune_counter);
      }
    }

    parameter_value_set original_values = this->f.get_parameter_values();

    tune_impl(args...);
    this->f.set_parameter_values(original_values);

    if (this->parameter_adjustment_functor) {
      this->parameter_adjustment_functor(optimal_parameters);
    }
    return optimal_parameters;
  }

  evaluate_t evaluate(Args &... args) {
    evaluations += 1;
    parameter_value_set original_kernel_values = f.get_parameter_values();
    bool do_evaluate =
        apply_parameters(f, parameters, parameter_adjustment_functor,
                         parameter_values_adjustment_functor, result_cache,
                         verbose); // does adjustment if applicable
    if (do_evaluate) {
      evaluate_t is_better =
          detail::evaluate_parameters<abstract_tuner, parameter_interface, R,
                                      Args...>(*this, f, parameters, args...);
      f.set_parameter_values(original_kernel_values);
      if (is_better != evaluate_t::skipped_failed) {
        evaluations_passed += 1;
      }
      return is_better;
    }
    return evaluate_t::skipped_failed;
  }

  bool evaluate_parallel(std::vector<parameter_interface> &parameters,
                         Args &... args) {
    std::vector<std::unique_ptr<
        autotune::abstract_kernel<R, cppjit::detail::pack<Args...>>>>
        kernels;
    std::vector<bool> do_evaluates;
    // clone all kernels and set its parameters
    for (size_t i = 0; i < parameters.size(); i++) {
      std::unique_ptr<
          autotune::abstract_kernel<R, cppjit::detail::pack<Args...>>>
          clone(f.clone());
      bool do_evaluate =
          apply_parameters(*clone, parameters[i], parameter_adjustment_functor,
                           parameter_values_adjustment_functor, result_cache,
                           verbose); // does adjustment if applicable
      do_evaluates.push_back(do_evaluate);
      kernels.push_back(std::move(clone));
    }

    int64_t no_to_evaluate =
        std::count(do_evaluates.begin(), do_evaluates.end(), true);
    if (no_to_evaluate == 0) {
      return false;
    }

    auto start = std::chrono::high_resolution_clock::now();
#pragma omp parallel for
    for (size_t i = 0; i < kernels.size(); i++) {
      if (do_evaluates[i]) {
        kernels[i]->compile();
      }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_parallel_compile = end - start;

    if (reporter) {
      reporter->write_compilation(duration_parallel_compile.count(),
                                  no_to_evaluate);
    }

    bool any_better = false;
    for (size_t i = 0; i < parameters.size(); i++) {
      if (do_evaluates[i]) {
        evaluate_t is_better =
            detail::evaluate_parameters<abstract_tuner, parameter_interface, R,
                                        Args...>(*this, *kernels[i],
                                                 parameters[i], args...);
        if (is_better == autotune::evaluate_t::better) {
          any_better = true;
        }
      }
    }
    return any_better;
  }

  void set_verbose(bool verbose) { this->verbose = verbose; }

  void set_write_measurement(const std::string &scenario_name) {
    write_measurement = true;
    this->scenario_name = scenario_name;
    // reporter = std::make_shared<csv_reporter>(
    //     scenario_name, to_parameter_values(parameters), tune_counter);
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

  // execute kernel multiple times to average across the result
  void set_repetitions(size_t repetitions) { this->repetitions = repetitions; }

  // reset cache and optimal for each run (group tuner disables this)
  void set_auto_clear(bool clear_tuner) { this->clear_tuner = clear_tuner; };

  virtual void reset_impl() = 0;

  parameter_interface &get_parameters() { return parameters; }

  evaluate_t update_parameters(parameter_interface adjusted_candidate,
                               double candidate_duration,
                               double duration_compile) {
    if (reporter) {
      reporter->write_measurement(to_parameter_values(adjusted_candidate),
                                  candidate_duration, candidate_duration,
                                  duration_compile);
    }
    evaluate_t is_better = evaluate_t::worse_equal;
    if (optimal_duration < 0.0 || candidate_duration < optimal_duration) {
      optimal_duration = candidate_duration;
      optimal_parameters = adjusted_candidate;
      std::cout << "new best kernel; duration: " << candidate_duration
                << std::endl;
      parameters.print_values();
      is_better = evaluate_t::better;
    }
    return is_better;
  }

  parameter_result_cache &get_result_cache() { return result_cache; }

  bool is_verbose() { return verbose; }

  size_t get_repetitions() { return repetitions; }

  int64_t get_evaluations() { return evaluations; }

  int64_t get_passed_evaluations() { return evaluations_passed; }
};

} // namespace autotune
