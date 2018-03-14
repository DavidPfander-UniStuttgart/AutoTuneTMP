#pragma once

#include <chrono>

#include "../util.hpp"
#include "abstract_tuner.hpp"
#include "with_tests.hpp"

namespace autotune {
namespace tuners {

// template <typename parameter_interface, typename R, typename... Args>
// group_tuners(
//     std::initializer_list<abstract_tuner<parameter_interface, R, Args...> *>
//         tuners) {}

template <typename parameter_interface, typename R, typename... Args>
class group_tuner {
  autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f;
  std::vector<
      std::reference_wrapper<abstract_tuner<parameter_interface, R, Args...>>>
      tuners;
  size_t group_repeat;
  bool verbose;

public:
  // group_tuner(
  //     autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
  //     size_t group_repeat,
  //     std::initializer_list<abstract_tuner<parameter_interface, R, Args...>
  //     *>
  //         tuners)
  //     : f(f), group_repeat(group_repeat) {
  //   // (this->tuners.push_back(tuners), ...);
  //   for (auto &t : tuners) {
  //     this->tuners.push_back(t);
  //   }
  //   // detail::iterate_tuple(this->tuners,
  //   //                       [](auto &t) { t.set_auto_clear(false); });
  // }

  // template argument only used to circumvent limitation of c++ "Us" all match
  // "R"
  // only way to implement a variable number of arguments
  // need to mention abstract_tuner to properly automatically infer
  // parameter_interface
  template <typename... Us>
  group_tuner(autotune::abstract_kernel<R, cppjit::detail::pack<Args...>> &f,
              size_t group_repeat,
              abstract_tuner<parameter_interface, Us, Args...> &... tuners)
      : f(f), group_repeat(group_repeat), verbose(false) {
    (this->tuners.push_back(tuners), ...);

    // make sure that parameter value caching across "tune" calls is enabled
    for (size_t i = 0; i < this->tuners.size(); i++) {
      this->tuners[i].get().set_auto_clear(false);
    }
  }

  parameter_value_set tune(Args &... args) {
    parameter_value_set original_values = this->f.get_parameter_values();

    for (size_t group_restart = 0; group_restart < group_repeat;
         group_restart++) {
      if (verbose) {
        std::cout << "group_restart: " << group_restart << std::endl;
      }

      for (size_t i = 0; i < tuners.size(); i++) {
        if (verbose) {
          std::cout << "tuner index: " << i << std::endl;
        }
        std::chrono::high_resolution_clock::time_point start =
            std::chrono::high_resolution_clock::now();
        tuners[i].get().tune(args...);
        std::chrono::high_resolution_clock::time_point end =
            std::chrono::high_resolution_clock::now();
        double tuning_duration =
            std::chrono::duration<double>(end - start).count();
        if (verbose) {
          std::cout << "tuner duration: " << tuning_duration << std::endl;
        }
        f.set_parameter_values(tuners[i].get().get_optimal_parameter_values());
      }
    }

    parameter_value_set optimal_parameter_values = f.get_parameter_values();
    if (verbose) {
      std::cout << "optimal parameter values (group tuner):" << std::endl;
      autotune::print_parameter_values(optimal_parameter_values);
    }
    f.set_parameter_values(original_values);
    return optimal_parameter_values;
  }

  void set_verbose(bool verbose) { this->verbose = verbose; }

  void setup_test(std::function<bool(R)> test_functional) {
    for (size_t i = 0; i < this->tuners.size(); i++) {
      this->tuners.setup_test(test_functional);
    }
  };
};
}
}
