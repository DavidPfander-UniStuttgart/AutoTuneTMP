#pragma once

#include "../abstract_kernel.hpp"
#include "../constraint_graph.hpp"
// #include "../simple_constraints.hpp"
#include "parameter_result_cache.hpp"
#include "with_tests.hpp"

#include <chrono>

namespace autotune {

template <typename parameter_interface, typename R, typename... Args>
class abstract_tuner;

// template <typename parameter_interface, typename R, typename... Args>
// bool evaluate_parameters(abstract_tuner<parameter_interface, R, Args...>
// &tuner,
//                          abstract_kernel<R, cppjit::detail::pack<Args...>>
//                          &f, parameter_interface &parameters, Args &... args)

namespace detail {
template <typename parameter_interface, typename R, typename... Args>
bool evaluate_parameters(abstract_tuner<parameter_interface, R, Args...> &tuner,
                         abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
                         parameter_interface &parameters, Args &... args);
}

template <typename parameter_interface, typename R, typename... Args>
class abstract_tuner
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
  bool do_measurement;
  bool do_write_header;
  std::ofstream scenario_file;
  // std::ofstream scenario_compile_duration_file;
  // std::ofstream scenario_parallel_compile_duration_file;

  parameter_result_cache result_cache;

  std::function<void(parameter_interface &)> parameter_adjustment_functor;
  std::function<void(parameter_value_set &)>
      parameter_values_adjustment_functor;

  // std::shared_ptr<simple_constraints> simple_constraints_wrapper;
  // std::shared_ptr<constraint_graph> constraint_graph_wrapper;

  size_t repetitions;

  bool clear_tuner;

  double parallel_compilation_duration_reporting;
  uint64_t parallel_compilation_count_reporting;

  virtual void tune_impl(Args &... args) = 0;

public:
  abstract_tuner(autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
                 parameter_interface &parameters)
      : f(f), parameters(parameters), optimal_duration(-1.0), verbose(false),
        do_measurement(false), do_write_header(true), repetitions(1),
        clear_tuner(true), parallel_compilation_duration_reporting(0.0),
        parallel_compilation_count_reporting(0) {}

  parameter_interface tune(Args &... args) {
    // if first run or auto clear is active
    if (clear_tuner || optimal_duration < 0) {
      result_cache.clear();
      optimal_duration = -1.0;
      optimal_parameters = parameters;
    }

    parameter_value_set original_values = this->f.get_parameter_values();

    tune_impl(args...);
    this->f.set_parameter_values(original_values);

    if (this->parameter_adjustment_functor) {
      this->parameter_adjustment_functor(optimal_parameters);
    }
    return optimal_parameters;
  }

  bool evaluate(Args &... args) {
    parameter_value_set original_kernel_values = f.get_parameter_values();
    bool do_evaluate =
        apply_parameters(f, parameters); // does adjustment if applicable
    if (do_evaluate) {
      bool found = detail::evaluate_parameters<parameter_interface, R, Args...>(
          *this, f, parameters, args...);
      f.set_parameter_values(original_kernel_values);
      return found;
    }
    return false;
  }

  // bool evaluate_cloned(Args &... args) {
  //   return evaluate_with_kernel(f->clone(), parameters, args...);
  // }

  bool evaluate_parallel(std::vector<parameter_interface> &parameters,
                         Args &... args) {
    parameter_value_set first_parameter_values =
        to_parameter_values(parameters[0]);
    write_header(first_parameter_values);

    std::vector<std::unique_ptr<
        autotune::abstract_kernel<R, cppjit::detail::pack<Args...>>>>
        kernels;
    std::vector<bool> do_evaluates;
    // clone all kernels and set its parameters
    for (size_t i = 0; i < parameters.size(); i++) {
      std::unique_ptr<
          autotune::abstract_kernel<R, cppjit::detail::pack<Args...>>>
          clone(f.clone());
      // clone->set_parameter_values(parameters[i]);
      bool do_evaluate = apply_parameters(
          *clone, parameters[i]); // does adjustment if applicable
      do_evaluates.push_back(do_evaluate);
      kernels.push_back(std::move(clone));
    }

    int64_t no_to_evaluate =
        std::count(do_evaluates.begin(), do_evaluates.end(), true);

    auto start = std::chrono::high_resolution_clock::now();
#pragma omp parallel for
    for (size_t i = 0; i < kernels.size(); i++) {
      if (do_evaluates[i]) {
        kernels[i]->compile();
      }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_parallel_compile = end - start;
    // scenario_file << no_to_evaluate << ", " <<
    // duration_parallel_compile.count()
    //               << std::endl;
    // nulled after begin written to file
    parallel_compilation_duration_reporting = duration_parallel_compile.count();
    parallel_compilation_count_reporting = no_to_evaluate;

    bool any_better = false;
    for (size_t i = 0; i < parameters.size(); i++) {
      if (do_evaluates[i]) {
        bool better =
            detail::evaluate_parameters<parameter_interface, R, Args...>(
                *this, *kernels[i], parameters[i], args...);
        if (!any_better && better) {
          any_better = true;
        }
      }
    }
    return any_better;
  }

  void set_verbose(bool verbose) { this->verbose = verbose; }

  void set_write_measurement(const std::string &scenario_name) {
    if (do_measurement) {
      if (scenario_file.is_open()) {
        scenario_file.close();
      }
      // if (scenario_compile_duration_file.is_open()) {
      //   scenario_compile_duration_file.close();
      // }
      // if (scenario_parallel_compile_duration_file.is_open()) {
      //   scenario_parallel_compile_duration_file.close();
      // }
    }
    do_measurement = true;
    do_write_header = true;
    scenario_file.open(scenario_name + "_kernel.csv");
    // scenario_compile_duration_file.open(scenario_name +
    // "_compile_duration.csv");
    // scenario_parallel_compile_duration_file.open(scenario_name +
    // "_par_comp_duration.csv");
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

  parameter_interface &get_parameters() { return parameters; }

  // void
  // set_simple_constraints(autotune::simple_constraints &my_simple_constraints)
  // {
  //   this->simple_constraints_wrapper =
  //       std::shared_ptr<autotune::simple_constraints>(&my_simple_constraints);
  // }

  // void set_constraint_graph(autotune::constraint_graph &my_constraint_graph)
  // {
  //   this->constraint_graph_wrapper =
  //       std::shared_ptr<autotune::constraint_graph>(&my_constraint_graph);
  // }

  // void
  // set_constraint_graph(autotune::simple_constraints
  // &constraint_graph_wrapper) {
  //   this->constraint_graph_wrapper = &constraint_graph_wrapper;
  // }

  // void
  // adjust_through_group_tuner(group_tuner<parameter_interface, R, Args...> &g)
  // {
  //   group_tuner_for_adjust = &g;
  // }

  bool update_parameters(parameter_interface adjusted_candidate,
                         double candidate_duration,
                         double candiate_duration_compile) {
    if (do_measurement) {
      parameter_value_set candidate_parameter_values =
          to_parameter_values(adjusted_candidate);
      write_measurement(candidate_parameter_values, candidate_duration,
                        candiate_duration_compile);
    }
    bool is_better = false;
    if (optimal_duration < 0.0 || candidate_duration < optimal_duration) {
      optimal_duration = candidate_duration;
      optimal_parameters = adjusted_candidate;
      report_verbose("new best kernel", optimal_duration, adjusted_candidate);
      is_better = true;
    }
    return is_better;
  }

  parameter_result_cache &get_result_cache() { return result_cache; }

  bool is_verbose() { return verbose; }

  void report(const std::string &message, double duration,
              parameter_interface &parameters) {
    std::cout << message << "; duration: " << duration << std::endl;
    parameters.print_values();
  }

  void report_verbose(const std::string &message, double duration,
                      parameter_interface &parameters) {
    if (verbose) {
      report(message, duration, parameters);
    }
  }

  void write_header(parameter_value_set parameter_values) {
    if (do_measurement && do_write_header) {
      // const parameter_value_set &parameter_values = f.get_parameter_values();
      bool first = true;
      for (auto &p : parameter_values) {
        if (!first) {
          scenario_file << ",";
          // scenario_compile_duration_file << ",";
        } else {
          first = false;
        }
        scenario_file << p.first;
        // scenario_compile_duration_file << p.first;
      }
      scenario_file << ","
                    << "kernel_s,compile_s,par_compile_s,par_compile_count"
                    << std::endl;
      // scenario_compile_duration_file << ","
      //                                << "duration" << std::endl;
      // scenario_parallel_compile_duration_file << "count, duration" <<
      // std::endl;
      do_write_header = false;
    }
  }

  void write_measurement(parameter_value_set parameter_values,
                         double duration_kernel_s, double duration_compile_s) {
    // const parameter_value_set &parameter_values = f.get_parameter_values();
    bool first = true;
    for (auto &p : parameter_values) {
      if (!first) {
        scenario_file << ",";
        // scenario_compile_duration_file << ",";
      } else {
        first = false;
      }
      scenario_file << p.second;
      // scenario_compile_duration_file << p.second;
    }
    scenario_file << "," << duration_kernel_s << "," << duration_compile_s
                  << "," << parallel_compilation_duration_reporting << ","
                  << parallel_compilation_count_reporting << std::endl;
    parallel_compilation_duration_reporting = 0.0;
    parallel_compilation_count_reporting = 0;
    // scenario_compile_duration_file << "," << duration_compile_s << std::endl;
  }

  size_t get_repetitions() { return repetitions; }

  // returns true if an apply is useful (adjusted and validated)
  bool apply_parameters(
      autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &kernel,
      const parameter_interface &parameters) {
    if (parameter_adjustment_functor) {
      parameter_interface adjusted = parameters;
      if (verbose) {
        std::cout << "------ parameters pre-adjustment ------" << std::endl;
        adjusted.print_values();
        std::cout << "--------------------------" << std::endl;
      }

      parameter_adjustment_functor(adjusted);

      if (verbose) {
        std::cout << "------ post-adjustment values ------" << std::endl;
        adjusted.print_values();
        std::cout << "--------------------------" << std::endl;
      }
      parameter_value_set adjusted_values = to_parameter_values(adjusted);
      if (!kernel.precompile_validate_parameters(adjusted_values)) {
        if (verbose) {
          std::cout << "------ invalidated eval (precompile) ------"
                    << std::endl;
          adjusted.print_values();
          std::cout << "--------------------------" << std::endl;
        }
        return false;
      } else {
        if (verbose) {
          std::cout << "parameter combination passed precompile check"
                    << std::endl;
        }
      }
      kernel.set_parameter_values(adjusted);
    } else if (parameter_values_adjustment_functor) {
      parameter_value_set adjusted = to_parameter_values(parameters);
      if (verbose) {
        std::cout << "------ parameters pre-adjustment ------" << std::endl;
        print_parameter_values(adjusted);
        std::cout << "--------------------------" << std::endl;
      }

      parameter_values_adjustment_functor(adjusted);

      if (verbose) {
        std::cout << "------ post-adjustment values ------" << std::endl;
        print_parameter_values(adjusted);
        std::cout << "--------------------------" << std::endl;
      }
      if (!kernel.precompile_validate_parameters(adjusted)) {
        if (verbose) {
          std::cout << "------ invalidated eval (precompile) ------"
                    << std::endl;
          print_parameter_values(adjusted);
          std::cout << "--------------------------" << std::endl;
        }
        return false;
      } else {
        if (verbose) {
          std::cout << "parameter combination passed precompile check"
                    << std::endl;
        }
      }
      kernel.set_parameter_values(adjusted);
    } else {
      if (verbose) {
        std::cout << "------ no adjustment functor ------" << std::endl;
      }
      parameter_value_set parameter_values = to_parameter_values(parameters);
      if (!kernel.precompile_validate_parameters(parameter_values)) {
        if (verbose) {
          std::cout << "------ invalidated eval (precompile) ------"
                    << std::endl;
          parameters.print_values();
          std::cout << "--------------------------" << std::endl;
        }
        return false;
      } else {
        if (verbose) {
          std::cout << "parameter combination passed precompile check"
                    << std::endl;
        }
      }
      kernel.set_parameter_values(parameters);
    }
    parameter_value_set kernel_values = kernel.get_parameter_values();
    // parameter_value_set parameter_values = f.get_parameter_values();
    if (!result_cache.contains(kernel_values)) {
      if (verbose) {
        std::cout << "------ add to cache ------" << std::endl;
        print_parameter_values(kernel_values);
      }
      result_cache.insert(kernel_values);
      return true;
    } else {
      if (verbose) {
        std::cout << "------ skipped eval ------" << std::endl;
        print_parameter_values(kernel_values);
        std::cout << "--------------------------" << std::endl;
      }
      return false;
    }
  }

}; // namespace autotune

namespace detail {

// returns whether evaluate lead to new optimal configuration found
template <typename parameter_interface, typename R, typename... Args>
bool evaluate_parameters(
    abstract_tuner<parameter_interface, R, Args...> &tuner,
    abstract_kernel<R, cppjit::detail::pack<Args...>> &kernel,
    parameter_interface &adjusted_parameters, Args &... args) {
  bool verbose = tuner.is_verbose();

  parameter_value_set adjusted_parameter_values =
      to_parameter_values(adjusted_parameters);

  tuner.write_header(adjusted_parameter_values);

  if (verbose) {
    std::cout << "------ begin eval ------" << std::endl;
    print_parameter_values(adjusted_parameter_values);
  }

  auto start_compile = std::chrono::high_resolution_clock::now();
  if (!kernel.is_compiled()) {
    kernel.compile();
  }
  auto end_compile = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration_compile = end_compile - start_compile;

  if (!kernel.is_valid_parameter_combination()) {
    if (verbose) {
      std::cout << "invalid parameter combination encountered" << std::endl;
    }
    return false;
  } else {
    if (verbose) {
      std::cout << "parameter combination is valid" << std::endl;
    }
  }

  auto start = std::chrono::high_resolution_clock::now();

  // call kernel, discard possibly returned values
  if constexpr (!std::is_same<R, void>::value) {
    if (tuner.has_test()) {
      for (size_t i = 0; i < tuner.get_repetitions(); i++) {
        bool test_ok = tuner.test(kernel(args...));
        if (!test_ok) {
          if (verbose) {
            std::cout << "warning: test for combination failed!" << std::endl;
          }
          return false;
        } else {
          if (verbose) {
            std::cout << "test for combination passed" << std::endl;
          }
        }
      }
    } else {
      for (size_t i = 0; i < tuner.get_repetitions(); i++) {
        kernel(args...);
      }
    }
  } else {
    for (size_t i = 0; i < tuner.get_repetitions(); i++) {
      kernel(args...);
    }
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration = end - start;
  size_t repetitions = tuner.get_repetitions();
  if (verbose) {
    if (kernel.has_kernel_duration_functor()) {
      double internal_duration = kernel.get_internal_kernel_duration();
      std::cout << "internal duration: " << internal_duration << std::endl;
      if (repetitions > 1) {
        std::cout << "internal duration per repetition: "
                  << (internal_duration / static_cast<double>(repetitions))
                  << std::endl;
      }
      std::cout << "(duration tuner: " << duration.count() << "s)" << std::endl;
      if (repetitions > 1) {
        std::cout << "(duration tuner per repetition: "
                  << (duration.count() / static_cast<double>(repetitions))
                  << "s)" << std::endl;
      }
    } else {
      std::cout << "duration: " << duration.count() << "s" << std::endl;
      if (repetitions > 1) {
        std::cout << "duration tuner per reptition: "
                  << (duration.count() / static_cast<double>(repetitions))
                  << "s" << std::endl;
      }
      std::cout << "------- end eval -------" << std::endl;
    }
  }

  double final_duration;
  if (kernel.has_kernel_duration_functor()) {
    final_duration = kernel.get_internal_kernel_duration();
  } else {
    final_duration = duration.count();
  }

  return tuner.update_parameters(adjusted_parameters, final_duration,
                                 duration_compile.count());
}

} // namespace detail

} // namespace autotune
