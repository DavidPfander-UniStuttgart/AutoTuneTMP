#pragma once

#include "parameter_result_cache.hpp"

namespace autotune {

// returns true if an apply is useful (adjusted and validated)
template <typename parameter_interface, typename R, typename... Args>
bool apply_parameters(
    autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &kernel,
    const parameter_interface &parameters,
    std::function<void(parameter_interface &)> parameter_adjustment_functor,
    std::function<void(parameter_value_set &)>
        parameter_values_adjustment_functor,
    parameter_result_cache &result_cache, bool verbose) {

  // consider parameter adjusts (by value or by parameter)
  parameter_interface adjusted = parameters;
  // merge already existing parameter values with current values
  // necessary to properly suppor value-adjustment of group tuners
  parameter_value_set adjusted_values = kernel.get_parameter_values();
  for (size_t i = 0; i < parameters.size(); i += 1) { //to_parameter_values(parameters);
      adjusted_values[parameters[i]->get_name()] = parameters[i]->get_value();
  }
  if (parameter_adjustment_functor) {
    if (verbose) {
      std::cout
          << "------ adjusting through parameter_adjustment_functor ------"
          << std::endl;
      std::cout << "------ parameters pre-adjustment ------" << std::endl;
      adjusted.print_values();
      std::cout << "--------------------------" << std::endl;
    }

    parameter_adjustment_functor(adjusted);
    adjusted_values = to_parameter_values(adjusted);

  } else if (parameter_values_adjustment_functor) {
    // only adjusts values, parameter set itself is left as it is
    parameter_value_set adjusted = to_parameter_values(parameters);
    if (verbose) {
      std::cout << "------ adjusting through "
                   "parameter_values_adjustment_functor ------"
                << std::endl;
      std::cout << "------ parameters pre-adjustment ------" << std::endl;
      print_parameter_values(adjusted);
      std::cout << "--------------------------" << std::endl;
    }

    parameter_values_adjustment_functor(adjusted_values);
  }

  if (verbose) {
    std::cout << "------ (post-adjustment) parameter values ------"
              << std::endl;
    adjusted.print_values();
    std::cout << "--------------------------" << std::endl;
  }

  // check result cache
  if (!result_cache.contains(adjusted_values)) {
    if (verbose) {
      std::cout << "info: new values, add to cache" << std::endl;
      print_parameter_values(adjusted_values);
    }
    result_cache.insert(adjusted_values);
  } else {
    if (verbose) {
      std::cout << "info: cached, skipped eval" << std::endl;
    }
    return false;
  }

  // validate parameter values (precompile, there is a post-compile hook as
  // well)
  if (!kernel.precompile_validate_parameters(adjusted_values)) {
    if (verbose) {
      std::cout << "info: invalidated eval (precompile)!" << std::endl;
    }
    return false;
  } else {
    if (verbose) {
      std::cout << "info: passed precompile check" << std::endl;
    }
  }
  kernel.set_parameter_values(adjusted_values);
  return true;
}

} // namespace autotune
