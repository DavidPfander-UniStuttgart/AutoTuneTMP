#pragma once

#include "../abstract_kernel.hpp"
#include "../parameter.hpp"
#include "countable_set.hpp"

#include <mutex>

namespace autotune {
namespace tuners {

class grid_line_search {
private:
  enum class phase { INITIAL, NEXT, PREV, WAITING_FOR_RESULTS };

  countable_set initial_parameters;
  countable_set cur_parameters;
  countable_set best_parameters;
  double best_parameters_duration;
  int64_t parameters_in_flight;
  size_t current_index;
  phase state;
  int64_t steps;
  int64_t current_step;

  bool verbose;

  std::shared_ptr<countable_parameter> cur_base_parameter;

  // protects best_parameters, parameters_in_flight
  std::mutex access_parameters_mutex;

public:
  grid_line_search(countable_set &parameters, int64_t steps,
                   bool verbose = false)
      : initial_parameters(parameters), cur_parameters(parameters),
        best_parameters(parameters), best_parameters_duration(-1.0),
        parameters_in_flight(0), current_index(0), state(phase::INITIAL),
        steps(steps), current_step(0), verbose(verbose) {
    if (verbose) {
      std::cout << "grid line search: creating grid line search" << std::endl;
    }
    if (cur_parameters.size() > 0) {
      cur_base_parameter = cur_parameters[0]->clone_wrapper();
    }
  }

  countable_set get_next(bool &still_tuning, bool &update) {
    std::unique_lock<std::mutex> lock(access_parameters_mutex);
    if (cur_parameters.size() == 0) {
      still_tuning = false;
      update = false;
      if (verbose) {
        std::cout << "grid line search: no parameters to tune, returning "
                     "initial parameters"
                  << std::endl;
      }
      return best_parameters;
    }

    if (current_step == steps) {
      still_tuning = false;
      update = false;
      if (verbose) {
        std::cout << "grid line search: search done, returning best parameters"
                  << std::endl;
      }
      return best_parameters;
    }

    // make sure to evaluate the initially supplied parameter combination
    if (state == phase::INITIAL) {
      state = phase::NEXT;
      still_tuning = true;
      update = true;
      parameters_in_flight += 1;
      std::cout << "grid line search: evaluating initial parameter combination:"
                << std::endl;
      cur_parameters.print_values();
      return cur_parameters;
    }

    while (current_step < steps) {
      if (state == phase::NEXT) {
        // std::cout << "cur_parameters.size(): " << cur_parameters.size() <<
        // std::endl;
        bool has_next_value = cur_parameters[current_index]->next();
        if (has_next_value) {
          still_tuning = true;
          update = true;
          parameters_in_flight += 1;
          if (verbose) {
            std::cout << "grid line search: changed name: "
                      << cur_parameters[current_index]->get_name() << " => "
                      << cur_parameters[current_index]->get_value()
                      << std::endl;
          }
          return cur_parameters;
        } else {
          // next_phase = false;
          state = phase::PREV;
          // cur_parameters[current_index]->set_initial();
          cur_parameters[current_index] = cur_base_parameter;
        }
      }

      if (state == phase::PREV) { // change back to PREV
        bool has_prev_value = cur_parameters[current_index]->prev();

        if (has_prev_value) {
          still_tuning = true;
          update = true;
          parameters_in_flight += 1;
          if (verbose) {
            std::cout << "grid line search: changed name: "
                      << cur_parameters[current_index]->get_name() << " => "
                      << cur_parameters[current_index]->get_value()
                      << std::endl;
          }
          return cur_parameters;
        } else {
          state = phase::WAITING_FOR_RESULTS;
        }
      }

      // there might be other combinations still in their evaluation phase,
      // wait until all updates from those combinations have been received
      if (state == phase::WAITING_FOR_RESULTS) {
        if (parameters_in_flight == 0) {
          if (verbose) {
            std::cout << "grid line search: all updates for this parameter "
                         "received, transitioning to next parameter"
                      << std::endl;
          }
          current_index += 1;
          // next_phase = true;
          state = phase::NEXT;
          // use the result of this parameter optimization as new starting point
          cur_parameters = best_parameters;
          if (current_index == cur_parameters.size()) {
            current_index = 0;
            current_step += 1;
          }
          // for following next phase
          cur_base_parameter = cur_parameters[current_index]->clone_wrapper();
        } else {
          if (verbose) {
            std::cout << "grid line search: waiting for remaining in-flight "
                         "candidates to update, remaining: "
                      << parameters_in_flight << std::endl;
          }

          still_tuning = true;
          update = false;
          return best_parameters;
        }
      }
    }
    if (verbose) {
      std::cout << "grid line search: no new candidates, search now ended"
                << std::endl;
    }
    {
      still_tuning = false;
      update = false;
      return best_parameters;
    }
  }

  countable_set get_best() {
    std::unique_lock<std::mutex> lock(access_parameters_mutex);
    return best_parameters;
  }

  void update_best(countable_set candidate, double candidate_duration) {
    std::unique_lock<std::mutex> lock(access_parameters_mutex);
    std::cout << "grid_line_search: update_best, candidate:" << std::endl;
    candidate.print_values();

    parameters_in_flight -= 1;
    if (candidate_duration < best_parameters_duration) {
      best_parameters = candidate;
      best_parameters_duration = candidate_duration;
    }
  }
};
} // namespace tuners
} // namespace autotune
