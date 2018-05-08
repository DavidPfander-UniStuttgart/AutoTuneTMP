#pragma once

#include "../abstract_kernel.hpp"
#include "../parameter.hpp"
#include "countable_set.hpp"

#include <mutex>

namespace autotune {
namespace tuners {

class grid_bruteforce {
private:
  countable_set initial_parameters;
  countable_set cur_parameters;
  countable_set best_parameters;
  double best_parameters_duration;
  bool first;
  size_t current_index;

  std::mutex access_parameters_mutex;

public:
  grid_bruteforce(countable_set &parameters)
      : initial_parameters(parameters), cur_parameters(parameters),
        best_parameters(parameters), best_parameters_duration(-1.0),
        first(true), current_index(0) {}

  countable_set get_next(bool &found) {
    std::unique_lock<std::mutex>(access_parameters_mutex);

    if (first) {
      // brute-force tuner
      for (size_t i = 0; i < this->cur_parameters.size(); i++) {
        this->cur_parameters[i]->set_min();
      }
      first = false;
      found = true;
      return cur_parameters;
    }

    while (true) {
      // left the range of valid indices, done!
      if (current_index == this->cur_parameters.size()) {
        found = false;
        return best_parameters;
      }

      // the is another value for the current parameter
      if (this->cur_parameters[current_index]->next()) {
        // reset the parameters "below" and start with the first parameter
        // again
        for (size_t i = 0; i < current_index; i++) {
          this->cur_parameters[i]->set_min();
        }
        current_index = 0;

        // evaluate new valid value vector
        found = true;
        return this->cur_parameters;
      } else {
        // no valid more values, try next parameter "above"
        current_index += 1;
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
} // namespace tuners
} // namespace autotune
