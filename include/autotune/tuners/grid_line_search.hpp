#pragma once

#include "../abstract_kernel.hpp"
#include "../parameter.hpp"
#include "countable_set.hpp"

#include <mutex>

namespace autotune {
namespace tuners {

class grid_line_search {
 private:
  countable_set initial_parameters;
  countable_set cur_parameters;
  countable_set best_parameters;
  double best_parameters_duration;
  bool first;
  size_t current_index;
  int64_t steps;
  int64_t current_step;
  bool next_phase;  // vs. prev phase

  std::mutex access_parameters_mutex;

 public:
  grid_line_search(countable_set &parameters, int64_t steps)
      : initial_parameters(parameters),
        cur_parameters(parameters),
        best_parameters(parameters),
        best_parameters_duration(-1.0),
        first(true),
        current_index(0),
        steps(steps),
        current_step(0),
        next_phase(true) {
    std::cout << "creating grid line search" << std::endl;
  }

  countable_set get_next(bool &found) {
    if (current_step == steps) {
      found = false;
      return cur_parameters;
    }

    std::unique_lock<std::mutex>(access_parameters_mutex);

    // make sure to evaluate the initially supplied parameter combination
    if (first) {
      first = false;
      found = true;
      return cur_parameters;
    }

    while (true) {
      if (next_phase) {
        bool has_next_value = cur_parameters[current_index]->next();
        if (has_next_value) {
          found = true;
          return cur_parameters;
        } else {
          next_phase = false;
          cur_parameters[current_index]->set_initial();
        }
      }

      bool has_prev_value = cur_parameters[current_index]->prev();

      if (has_prev_value) {
        found = true;
        return cur_parameters;
      } else {
        current_index += 1;
        next_phase = true;
        if (current_index == cur_parameters.size()) {
          current_index = 0;
          steps += 1;
        }
        // for following next phase
        cur_parameters[current_index]->set_initial();
      }
    }
  }

  countable_set get_best() {
    std::unique_lock<std::mutex>(access_parameters_mutex);
    return best_parameters;
  }

  void update_best(countable_set candidate, double candidate_duration) {
    std::unique_lock<std::mutex>(access_parameters_mutex);
    if (candidate_duration < best_parameters_duration) {
      best_parameters = candidate;
      best_parameters_duration = candidate_duration;
    }
  }
};
}  // namespace tuners
}  // namespace autotune
