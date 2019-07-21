#pragma once

#include "abstract_tuner.hpp"
#include "countable_set.hpp"
#include "parameter_result_cache.hpp"

#include <random>

namespace autotune {
namespace tuners {

template <typename R, typename... Args>
class parallel_full_neighborhood_search
    : public abstract_tuner<countable_set, R, Args...> {
private:
  size_t max_iterations;

public:
  parallel_full_neighborhood_search(
      autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
      countable_set &parameters, size_t max_iterations)
      : abstract_tuner<countable_set, R, Args...>(f, parameters),
        max_iterations(max_iterations) {}

private:
  virtual void tune_impl(Args &... args) override {

    bool is_initial = true;

    for (size_t i = 0; i < max_iterations; i++) {
      if (this->verbose) {
        std::cout << "--------- next iteration: " << i << " ---------"
                  << std::endl;
      }

			std::vector<countable_set> parameters_to_evaluate;
      if (is_initial) {
        parameters_to_evaluate.push_back(this->parameters);
        is_initial = false;
      }

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

      // initial parameter combination occurs as cur_offsets[i] = 0 for all i;
      // only computed once due to cache


      // iterate hypercube and collect all valid grid points (= parameter
      // combinations)
      while (true) {
        if (cur_index == this->parameters.size()) {
          break;
        }

        // next() increments parameter and checks whether still within bounds
        if (cur_offsets[cur_index] < 1 && this->parameters[cur_index]->next()) {
          cur_offsets[cur_index] += 1;

          for (size_t k = 0; k < cur_index; k++) {
            // resets parameters
            // prev() won't do anything if result would be out of bounds
            // if was 1 -> 0; if 0 -> -1, cannot have been -1 as 0 is always
            // valid
            if (cur_offsets[k] > -1) {
              if (this->parameters[k]->prev()) {
                cur_offsets[k] -= 1;
              }
            }
            // if was at 1, now at 0, attempt -1
            if (cur_offsets[k] == 0) {
              if (this->parameters[k]->prev()) {
                cur_offsets[k] -= 1;
              }
            }
          }
          cur_index = 0;

          // std::cout << "cur_offsets: ";
          // for (size_t i = 0; i < cur_offsets.size(); i += 1) {
          //   if (i > 0) {
          //     std::cout << ", ";
          //   }
          //   std::cout << cur_offsets[i];
          // }
          // std::cout << std::endl;
          // this->parameters.print_values();

          parameters_to_evaluate.push_back(this->parameters);
        } else {
          cur_index += 1;
        }
      }

      this->evaluate_parallel(parameters_to_evaluate, args...);
      this->parameters = this->optimal_parameters;
    }
  }

  void reset_impl() override {}
};

} // namespace tuners
} // namespace autotune
