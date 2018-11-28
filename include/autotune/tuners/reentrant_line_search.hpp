#pragma once

#include "abstract_reentrant_tuner.hpp"
#include "countable_set.hpp"
#include <random>

namespace autotune {
namespace tuners {

template <typename R, typename... Args>
class reentrant_line_search
    : public abstract_reentrant_tuner<countable_set, R, Args...> {
private:
  size_t max_iterations;
  int64_t last_updated;
  size_t iteration;
  size_t cur_index;
  enum class state_t {
    INIT,
    NEXT_PARAMETER,
    RUN_UPDATE_INITIAL,
    RUN_UPDATE_NEXT,
    RUN_UPDATE_PREV,
    DONE
  };
  state_t state;

public:
  reentrant_line_search(
      autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
      countable_set &parameters, size_t max_iterations)
      : abstract_reentrant_tuner<countable_set, R, Args...>(f, parameters),
        max_iterations(max_iterations), last_updated(-1), iteration(0),
        cur_index(0), state(state_t::INIT) {}

private:
  // only allow void-return kernels for now
  virtual void tune_impl(Args &... args) override {
    while (true) {
      if (state == state_t::INIT) {
        if (this->verbose) {
          std::cout << "info: state is INIT" << std::endl;
        }
        iteration = 0;
        cur_index = 0;
        last_updated = -1;
        state = state_t::NEXT_PARAMETER;
      } else if (state == state_t::NEXT_PARAMETER) {
        if (this->verbose) {
          std::cout << "info: state is NEXT_PARAMETER" << std::endl;
        }
        if (iteration < max_iterations &&
            iteration - last_updated < this->parameters.size()) {
          auto &p = this->parameters[cur_index];
          p->set_initial();
          autotune::evaluate_t is_better = this->evaluate(args...);
          if (is_better == autotune::evaluate_t::better) {
            last_updated = iteration;
          }
          state = state_t::RUN_UPDATE_NEXT;
          if (is_better != evaluate_t::skipped_failed) {
            return;
          }
        } else {
          this->parameters = this->optimal_parameters;
          this->f.set_parameter_values(this->optimal_parameters);
          if (this->verbose) {
            std::cout << "info: tuning phase finished" << std::endl;
            std::cout << "info: iterations: " << iteration
                      << " evaluations: " << this->get_evaluations()
                      << " evaluations_passed: "
                      << this->get_passed_evaluations() << std::endl;
          }
          state = state_t::DONE;
        }
      } else if (state == state_t::RUN_UPDATE_NEXT) {
        if (this->verbose) {
          std::cout << "info: state is RUN_UPDATE_NEXT" << std::endl;
        }
        auto &p = this->parameters[cur_index];
        if (p->next()) {
          autotune::evaluate_t is_better = this->evaluate(args...);
          if (is_better == autotune::evaluate_t::better) {
            last_updated = iteration;
          }
          if (is_better != evaluate_t::skipped_failed) {
            return;
          }
        } else {
          p->set_initial();
          state = state_t::RUN_UPDATE_PREV;
        }
      } else if (state == state_t::RUN_UPDATE_PREV) {
        if (this->verbose) {
          std::cout << "info: state is RUN_UPDATE_PREV" << std::endl;
        }
        auto &p = this->parameters[cur_index];
        if (p->next()) {
          autotune::evaluate_t is_better = this->evaluate(args...);
          if (is_better == autotune::evaluate_t::better) {
            last_updated = iteration;
          }
          if (is_better != evaluate_t::skipped_failed) {
            return;
          }
        } else {
          cur_index = (cur_index + 1) % this->parameters.size();
          iteration += 1;
          this->parameters = this->optimal_parameters;
          state = state_t::NEXT_PARAMETER;
        }
      } else if (state == state_t::DONE) {
        if (this->verbose) {
          std::cout << "info: state is DONE" << std::endl;
        }
        if (this->verbose) {
          std::cout << "info: call after tuning phase" << std::endl;
        }
        // this->evaluate(args...);
        this->f(args...);
        return;
      }
    }
  }

  void reset_impl() override { state = state_t::INIT; }
}; // namespace tuners

} // namespace tuners
} // namespace autotune
