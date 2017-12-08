#define BOOST_TEST_MODULE autotunetmp_boost_tests
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "autotune/autotune.hpp"
#include "autotune/tuners/bruteforce.hpp"
#include "autotune/tuners/countable_set.hpp"
#include "autotune/tuners/line_search.hpp"

AUTOTUNE_DECLARE_DEFINE_KERNEL(int(int), run_kernel)

AUTOTUNE_DECLARE_DEFINE_KERNEL(int(int), run_bruteforce_kernel)

AUTOTUNE_DECLARE_DEFINE_KERNEL(int(int), run_line_search_kernel)

BOOST_AUTO_TEST_SUITE(basic_api)

BOOST_AUTO_TEST_CASE(run_kernel) {
  autotune::run_kernel.set_source_dir("tests/kernel_run_kernel");
  autotune::run_kernel.set_verbose(true);
  int result = autotune::run_kernel(2);
  BOOST_CHECK_EQUAL(result, 5);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(bruteforce)

BOOST_AUTO_TEST_CASE(run_bruteforce) {
  autotune::run_bruteforce_kernel.set_source_dir(
      "tests/kernel_run_bruteforce_kernel");
  // autotune::run_bruteforce_kernel.set_verbose(true);
  auto builder =
      autotune::run_bruteforce_kernel.get_builder_as<cppjit::builder::gcc>();
  builder->set_cpp_flags("-Wall -Wextra -std=c++17 -fPIC");

  autotune::countable_set parameters;
  autotune::fixed_set_parameter p1("PAR_1", {"eins", "zwei"});
  parameters.add_parameter(p1);
  autotune::countable_continuous_parameter p2("PAR_2", 1.0, 1.0, 1.0, 3.0);
  parameters.add_parameter(p2);

  std::function<bool(int)> test_result = [](int) -> bool { return true; };

  int a = 5;

  autotune::tuners::bruteforce<decltype(autotune::run_bruteforce_kernel)> tuner(
      autotune::run_bruteforce_kernel, parameters);
  tuner.setup_test(test_result);
  tuner.set_verbose(true);
  autotune::countable_set optimal_parameters = tuner.tune(a);
  bool check1 = optimal_parameters[0]->get_value().compare("\"eins\"") == 0;
  BOOST_CHECK(check1);
  bool check2 = optimal_parameters[1]->get_value().compare("2.000000") == 0;
  BOOST_CHECK(check2);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(line_search)

BOOST_AUTO_TEST_CASE(run_line_search) {
  autotune::run_line_search_kernel.set_source_dir(
      "tests/kernel_run_line_search_kernel");
  // autotune::run_line_search_kernel.set_verbose(true);
  auto builder =
      autotune::run_line_search_kernel.get_builder_as<cppjit::builder::gcc>();
  builder->set_cpp_flags("-Wall -Wextra -std=c++17 -fPIC");

  autotune::countable_set parameters;
  autotune::fixed_set_parameter p1("PAR_1", {"eins", "zwei", "drei"});
  parameters.add_parameter(p1);
  autotune::countable_continuous_parameter p2("PAR_2", 1.0, 1.0, 1.0, 3.0);
  parameters.add_parameter(p2);

  std::function<bool(int)> test_result = [](int) -> bool { return true; };

  int a = 5;

  autotune::tuners::line_search<decltype(autotune::run_line_search_kernel)>
      tuner(autotune::run_line_search_kernel, 5, 1, parameters);
  tuner.setup_test(test_result);
  tuner.set_verbose(true);
  autotune::countable_set optimal_parameters = tuner.tune(a);
  bool check1 = optimal_parameters[0]->get_value().compare("\"zwei\"") == 0;
  BOOST_CHECK(check1);
  bool check2 = optimal_parameters[1]->get_value().compare("2.000000") == 0;
  BOOST_CHECK(check2);
}

BOOST_AUTO_TEST_SUITE_END()
