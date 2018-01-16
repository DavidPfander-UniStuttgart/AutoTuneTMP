#pragma once

#include "../autotune.hpp"
#include "../parameter.hpp"
#include "abstract_tuner.hpp"
#include "countable_set.hpp"

namespace autotune {
namespace tuners {

template <typename R, typename... Args>
class full_neighborhood_search
    : public abstract_tuner<countable_set, R, Args...> {
private:
  size_t iterations;

public:
  full_neighborhood_search(
      autotune::cppjit_kernel<R, cppjit::detail::pack<Args...>> &f,
      countable_set &parameters, size_t iterations)
      : abstract_tuner<countable_set, R, Args...>(f, parameters),
        iterations(iterations) {}

  countable_set tune(Args &... args) {

    this->result_cache.clear();

    parameter_value_set original_values = this->f.get_parameter_values();

    bool is_valid = true;
    double optimal_duration = -1.0;
    // double duration = this->evaluate(is_valid, args...);
    countable_set optimal_parameters = this->parameters;
    // if (is_valid) {
    //   optimal_parameters = this->parameters;
    //   optimal_duration = duration;
    // }

    for (size_t i = 0; i < iterations; i++) {
      if (this->verbose) {
        std::cout << "--------- next iteration: " << i << " ---------"
                  << std::endl;
      }
      // countable_set base_parameters = optimal_parameters;
      this->parameters.print_values();

      // initialize to first parameter combination and evaluate it
      size_t cur_index = 0;
      std::vector<int64_t> cur_offsets(this->parameters.size());

      for (size_t m = 0; m < this->parameters.size(); m++) {
        if (this->parameters[m]->prev()) {
          cur_offsets[m] = -1;
        } else {
          cur_offsets[m] = 0;
        }
      }

      double duration = this->evaluate(is_valid, args...);
      if (is_valid &&
          (optimal_duration == -1.0 || duration < optimal_duration)) {
        optimal_parameters = this->parameters;
        optimal_duration = duration;
        this->report_verbose("new best kernel", optimal_duration,
                             this->parameters);
      }

      while (true) {
        if (cur_index == this->parameters.size()) {
          break;
        }

        // for (size_t m = 0; m < cur_offsets.size(); m++) {
        //   if (m > 0) {
        //     std::cout << ", ";
        //   }
        //   std::cout << cur_offsets[m];
        // }
        // std::cout << std::endl;

        // next() increments parameter and checks whether still within bounds
        if (cur_offsets[cur_index] < 1 && this->parameters[cur_index]->next()) {
          // std::cout << "index found: " << cur_offsets[cur_index]
          //           << " at index: " << cur_index << std::endl;
          cur_offsets[cur_index] += 1;

          for (size_t k = 0; k < cur_index; k++) {
            // resets parameters
            // prev() won't do anything if result would be out of bounds
            if (this->parameters[k]->prev()) {
              cur_offsets[k] -= 1;
            }
            if (this->parameters[k]->prev()) {
              cur_offsets[k] -= 1;
            }
          }
          cur_index = 0;

          duration = this->evaluate(is_valid, args...);
          if (is_valid &&
              (optimal_duration == -1.0 || duration < optimal_duration)) {
            optimal_parameters = this->parameters;
            optimal_duration = duration;
            this->report_verbose("new best kernel", optimal_duration,
                                 this->parameters);
          }
        } else {
          // std::cout << "!!! no index found at: " << cur_index << " (inc
          // index)"
          //           << std::endl;
          cur_index += 1;
        }
      }

      this->parameters = optimal_parameters;
    }

    this->f.set_parameter_values(original_values);
    return optimal_parameters;
  }
};
} // namespace tuners
} // namespace autotune
