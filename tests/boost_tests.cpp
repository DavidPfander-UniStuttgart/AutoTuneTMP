#define BOOST_TEST_MODULE autotunetmp_boost_tests
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <thread>

#include "autotune/autotune.hpp"
#include "autotune/generalized_kernel.hpp"
#include "autotune/parameter_value_set.hpp"
#include "autotune/tuners/bruteforce.hpp"
#include "autotune/tuners/countable_set.hpp"
#include "autotune/tuners/full_neighborhood_search.hpp"
#include "autotune/tuners/line_search.hpp"
#include "autotune/tuners/monte_carlo.hpp"
#include "autotune/tuners/neighborhood_search.hpp"
#include "autotune/tuners/parallel_full_neighborhood_search.hpp"
#include "autotune/tuners/parallel_line_search.hpp"

AUTOTUNE_KERNEL(int(int), run_kernel, "tests/kernel_run_kernel")

AUTOTUNE_KERNEL(int(int), run_different_parameter_values,
                "tests/kernel_run_different_parameter_values")

AUTOTUNE_KERNEL(int(int), run_bruteforce_kernel,
                "tests/kernel_run_bruteforce_kernel")

AUTOTUNE_KERNEL(int(int), run_line_search_kernel,
                "tests/kernel_run_line_search_kernel")

AUTOTUNE_KERNEL(int(int), run_neighborhood_search_kernel,
                "tests/kernel_run_neighborhood_search_kernel")

AUTOTUNE_KERNEL(int(int), run_full_neighborhood_search_kernel,
                "tests/kernel_run_full_neighborhood_search_kernel")

AUTOTUNE_KERNEL(int(int), run_monte_carlo_kernel,
                "tests/kernel_run_monte_carlo_kernel")

AUTOTUNE_GENERALIZED_KERNEL(double(double), generalized_test_kernel)

BOOST_AUTO_TEST_SUITE(basic_api)

BOOST_AUTO_TEST_CASE(run_kernel) {
  autotune::run_kernel.set_verbose(true);
  int result = autotune::run_kernel(2);
  autotune::run_kernel.clear();
  BOOST_CHECK_EQUAL(result, 5);
}

BOOST_AUTO_TEST_CASE(factor_parameter) {
  autotune::countable_continuous_parameter p1("PAR_1", 1.0, 1.0, 1.0, 5.0,
                                              false);
  BOOST_CHECK(p1.count_values() == 5);
  autotune::countable_continuous_parameter p2("PAR_1", 1.0, 1.0, 1.0, 4.5,
                                              false);
  BOOST_CHECK(p2.get_value().compare("1") == 0);
  BOOST_CHECK(p2.count_values() == 4);
  BOOST_CHECK(p2.get_value().compare("1") == 0);
  p2.next();
  BOOST_CHECK(p2.get_value().compare("2") == 0);
  p2.set_min();
  p2.next();
  BOOST_CHECK(p2.get_value().compare("2") == 0);
  p2.next();
  BOOST_CHECK(p2.get_value().compare("3") == 0);
  p2.next();
  BOOST_CHECK(p2.get_value().compare("4") == 0);
  p2.next();
  BOOST_CHECK(p2.get_value().compare("4") == 0);
  p2.prev();
  BOOST_CHECK(p2.get_value().compare("3") == 0);
  p2.set_min();
  p2.prev();
  BOOST_CHECK(p2.get_value().compare("1") == 0);
  autotune::countable_continuous_parameter p3("PAR_2", 1.0, 2.0, 2.0, 10.0,
                                              true);
  BOOST_CHECK(p3.get_value().compare("2") == 0);
  BOOST_CHECK(p3.count_values() == 5);
  BOOST_CHECK(p3.get_value().compare("2") == 0);
  p3.next();
  BOOST_CHECK(p3.get_value().compare("4") == 0);
  p3.set_min();
  p3.next();
  std::cout << p3.get_value() << std::endl;
  BOOST_CHECK(p3.get_value().compare("4") == 0);
  p3.next();
  BOOST_CHECK(p3.get_value().compare("6") == 0);
  p3.next();
  BOOST_CHECK(p3.get_value().compare("8") == 0);
  p3.next();
  BOOST_CHECK(p3.get_value().compare("10") == 0);
  p3.prev();
  BOOST_CHECK(p3.get_value().compare("8") == 0);
  p3.set_min();
  p3.prev();
  BOOST_CHECK(p3.get_value().compare("2") == 0);
  autotune::countable_continuous_parameter p4("PAR_2", 1.0, 2.0, 2.0, 9.5,
                                              true);
  BOOST_CHECK(p4.count_values() == 4);
}

BOOST_AUTO_TEST_CASE(run_different_parameter_values) {
  autotune::countable_set parameters;
  autotune::fixed_set_parameter<int> p1("PAR_1", {1, 2});
  parameters.add_parameter(p1);

  autotune::run_different_parameter_values.set_verbose(true);
  // auto &builder = autotune::run_different_parameter_values
  //                     .get_builder<cppjit::builder::gcc>();
  // builder.set_verbose(true);

  // run with "PAR_1" set to "1"
  autotune::run_different_parameter_values.set_parameter_values(parameters);
  autotune::run_different_parameter_values.create_parameter_file();
  autotune::run_different_parameter_values.compile();
  int result = autotune::run_different_parameter_values(2);
  BOOST_CHECK_EQUAL(result, 1);
  // run with "PAR_1" set to "2"
  // change parameter value
  parameters[0]->next();

  autotune::run_different_parameter_values.set_parameter_values(parameters);
  autotune::run_different_parameter_values.create_parameter_file();
  autotune::run_different_parameter_values.compile();
  result = autotune::run_different_parameter_values(2);
  BOOST_CHECK_EQUAL(result, 2);
}

BOOST_AUTO_TEST_CASE(test_tuner_result_cache) {
  autotune::countable_set parameters;
  autotune::fixed_set_parameter<int> p1("PAR_1", {1, 2});
  autotune::fixed_set_parameter<int> p2("PAR_2", {3, 4});
  autotune::fixed_set_parameter<int> p3("PAR_3", {5, 6});

  parameters.add_parameter(p1);
  parameters.add_parameter(p2);
  parameters.add_parameter(p3);

  autotune::parameter_result_cache r;
  BOOST_CHECK(!r.contains(parameters));
  r.insert(parameters);
  BOOST_CHECK(r.contains(parameters));

  parameters[2]->next();
  BOOST_CHECK(!r.contains(parameters));
  r.insert(parameters);
  BOOST_CHECK(r.contains(parameters));

  parameters[0]->next();
  parameters[2]->prev();
  BOOST_CHECK(!r.contains(parameters));
  r.insert(parameters);
  BOOST_CHECK(r.contains(parameters));

  r.clear();
  BOOST_CHECK(!r.contains(parameters));
}

BOOST_AUTO_TEST_CASE(run_different_parameter_values_more_automation) {
  autotune::countable_set parameters;
  autotune::fixed_set_parameter<int> p1("PAR_1", {1, 2});
  parameters.add_parameter(p1);

  autotune::run_different_parameter_values.set_verbose(true);
  // auto &builder = autotune::run_different_parameter_values
  //                     .get_builder<cppjit::builder::gcc>();
  // builder.set_verbose(true);

  // run with "PAR_1" set to "1"
  autotune::run_different_parameter_values.set_parameter_values(parameters);
  int result = autotune::run_different_parameter_values(2);
  BOOST_CHECK_EQUAL(result, 1);
  // run with "PAR_1" set to "2"

  // autotune::run_different_parameter_values.clear();

  // change parameter value
  parameters[0]->next();

  autotune::run_different_parameter_values.set_parameter_values(parameters);
  result = autotune::run_different_parameter_values(2);
  BOOST_CHECK_EQUAL(result, 2);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(bruteforce)

BOOST_AUTO_TEST_CASE(run_bruteforce) {
  autotune::run_bruteforce_kernel.set_verbose(true);
  auto &builder =
      autotune::run_bruteforce_kernel.get_builder<cppjit::builder::gcc>();
  // builder.set_verbose(true);
  builder.set_cpp_flags("-Wall -Wextra -std=c++17 -fPIC");

  autotune::countable_set parameters;
  autotune::fixed_set_parameter<std::string> p1("PAR_1", {"eins", "zwei"});
  parameters.add_parameter(p1);
  autotune::countable_continuous_parameter p2("PAR_2", 1.0, 1.0, 1.0, 3.0);
  parameters.add_parameter(p2);
  autotune::fixed_set_parameter<int64_t> p3("PAR_3", {1});
  parameters.add_parameter(p3);
  autotune::fixed_set_parameter<double> p4("PAR_4", {2.0});
  parameters.add_parameter(p4);

  std::function<bool(int)> test_result = [](int) -> bool { return true; };

  autotune::tuners::bruteforce tuner(autotune::run_bruteforce_kernel,
                                     parameters);
  tuner.setup_test(test_result);
  tuner.set_verbose(true);

  int a = 5;
  autotune::countable_set optimal_parameters = tuner.tune(a);

  bool check1 = optimal_parameters[0]->get_value().compare("\"eins\"") == 0;
  BOOST_CHECK(check1);
  bool check2 = optimal_parameters[1]->get_value().compare("2") == 0;
  BOOST_CHECK(check2);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(line_search)

BOOST_AUTO_TEST_CASE(run_line_search) {
  // autotune::run_line_search_kernel.set_verbose(true);
  auto &builder =
      autotune::run_line_search_kernel.get_builder<cppjit::builder::gcc>();
  builder.set_cpp_flags("-Wall -Wextra -std=c++17 -fPIC");

  autotune::countable_set parameters;
  autotune::fixed_set_parameter<std::string> p1("PAR_1",
                                                {"eins", "zwei", "drei"});
  parameters.add_parameter(p1);
  autotune::countable_continuous_parameter p2("PAR_2", 1.0, 1.0, 1.0, 3.0);
  parameters.add_parameter(p2);

  std::function<bool(int)> test_result = [](int) -> bool { return true; };

  int a = 5;

  autotune::tuners::line_search tuner(autotune::run_line_search_kernel,
                                      parameters, 5);
  tuner.setup_test(test_result);
  tuner.set_verbose(true);
  autotune::countable_set optimal_parameters = tuner.tune(a);
  std::cout << optimal_parameters[0]->get_value() << std::endl;
  std::cout << optimal_parameters[1]->get_value() << std::endl;
  bool check1 = optimal_parameters[0]->get_value().compare("\"zwei\"") == 0;
  BOOST_CHECK(check1);
  bool check2 = optimal_parameters[1]->get_value().compare("2") == 0;
  BOOST_CHECK(check2);
}

BOOST_AUTO_TEST_CASE(run_parallel_line_search) {
  // autotune::run_line_search_kernel.set_verbose(true);
  auto &builder =
      autotune::run_line_search_kernel.get_builder<cppjit::builder::gcc>();
  builder.set_cpp_flags("-Wall -Wextra -std=c++17 -fPIC");

  autotune::countable_set parameters;
  autotune::fixed_set_parameter<std::string> p1("PAR_1",
                                                {"eins", "zwei", "drei"});
  parameters.add_parameter(p1);
  autotune::countable_continuous_parameter p2("PAR_2", 1.0, 1.0, 1.0, 3.0);
  parameters.add_parameter(p2);

  std::function<bool(int)> test_result = [](int) -> bool { return true; };

  int a = 5;

  autotune::tuners::parallel_line_search tuner(autotune::run_line_search_kernel,
                                               parameters, 5);
  tuner.setup_test(test_result);
  tuner.set_verbose(true);
  autotune::countable_set optimal_parameters = tuner.tune(a);
  std::cout << optimal_parameters[0]->get_value() << std::endl;
  std::cout << optimal_parameters[1]->get_value() << std::endl;
  bool check1 = optimal_parameters[0]->get_value().compare("\"zwei\"") == 0;
  BOOST_CHECK(check1);
  bool check2 = optimal_parameters[1]->get_value().compare("2") == 0;
  BOOST_CHECK(check2);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(neighborhood_search)

BOOST_AUTO_TEST_CASE(run_neighborhood_search) {
  autotune::run_neighborhood_search_kernel.set_verbose(true);
  auto &builder = autotune::run_neighborhood_search_kernel
                      .get_builder<cppjit::builder::gcc>();
  builder.set_cpp_flags("-Wall -Wextra -std=c++17 -fPIC");

  autotune::countable_set parameters;
  autotune::countable_continuous_parameter p1("PAR_1", 1.0, 1.0, 1.0, 5.0);
  parameters.add_parameter(p1);
  autotune::countable_continuous_parameter p2("PAR_2", 1.0, 1.0, 1.0, 5.0);
  parameters.add_parameter(p2);

  std::function<bool(int)> test_result = [](int) -> bool { return true; };

  int a = 5;

  autotune::tuners::neighborhood_search tuner(
      autotune::run_neighborhood_search_kernel, parameters, 5);
  tuner.setup_test(test_result);
  tuner.set_verbose(true);
  autotune::countable_set optimal_parameters = tuner.tune(a);
  optimal_parameters.print_values();
  bool check1 = optimal_parameters[0]->get_value().compare("2") == 0;
  BOOST_CHECK(check1);
  bool check2 = optimal_parameters[1]->get_value().compare("3") == 0;
  BOOST_CHECK(check2);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(full_neighborhood_search)

BOOST_AUTO_TEST_CASE(run_full_neighborhood_search) {
  autotune::run_full_neighborhood_search_kernel.set_verbose(true);
  auto &builder = autotune::run_full_neighborhood_search_kernel
                      .get_builder<cppjit::builder::gcc>();
  // builder.set_verbose(false);
  builder.set_cpp_flags("-Wall -Wextra -std=c++17 -fPIC");

  autotune::countable_set parameters;
  autotune::countable_continuous_parameter p1("PAR_1", 1.0, 1.0, 1.0, 5.0);
  parameters.add_parameter(p1);
  autotune::countable_continuous_parameter p2("PAR_2", 1.0, 1.0, 1.0, 5.0);
  parameters.add_parameter(p2);

  std::function<bool(int)> test_result = [](int) -> bool { return true; };

  int a = 5;

  autotune::tuners::full_neighborhood_search tuner(
      autotune::run_full_neighborhood_search_kernel, parameters, 10);
  tuner.setup_test(test_result);
  tuner.set_verbose(true);
  autotune::countable_set optimal_parameters = tuner.tune(a);
  optimal_parameters.print_values();
  bool check1 = optimal_parameters[0]->get_value().compare("1") == 0;
  BOOST_CHECK(check1);
  bool check2 = optimal_parameters[1]->get_value().compare("3") == 0;
  BOOST_CHECK(check2);
}

BOOST_AUTO_TEST_CASE(run_parallel_full_neighborhood_search) {
  autotune::run_full_neighborhood_search_kernel.set_verbose(true);
  auto &builder = autotune::run_full_neighborhood_search_kernel
                      .get_builder<cppjit::builder::gcc>();
  // builder.set_verbose(false);
  builder.set_cpp_flags("-Wall -Wextra -std=c++17 -fPIC");

  autotune::countable_set parameters;
  autotune::countable_continuous_parameter p1("PAR_1", 1.0, 1.0, 1.0, 5.0);
  parameters.add_parameter(p1);
  autotune::countable_continuous_parameter p2("PAR_2", 1.0, 1.0, 1.0, 5.0);
  parameters.add_parameter(p2);

  std::function<bool(int)> test_result = [](int) -> bool { return true; };

  int a = 5;

  autotune::tuners::parallel_full_neighborhood_search tuner(
      autotune::run_full_neighborhood_search_kernel, parameters, 10);
  tuner.setup_test(test_result);
  tuner.set_verbose(true);
  autotune::countable_set optimal_parameters = tuner.tune(a);
  optimal_parameters.print_values();
  bool check1 = optimal_parameters[0]->get_value().compare("1") == 0;
  BOOST_CHECK(check1);
  bool check2 = optimal_parameters[1]->get_value().compare("3") == 0;
  BOOST_CHECK(check2);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(monte_carlo_search)

BOOST_AUTO_TEST_CASE(run_monte_carlo) {
  autotune::run_monte_carlo_kernel.set_verbose(true);
  auto &builder =
      autotune::run_monte_carlo_kernel.get_builder<cppjit::builder::gcc>();
  builder.set_cpp_flags("-Wall -Wextra -std=c++17 -fPIC");

  autotune::randomizable_set parameters;
  autotune::limited_continuous_parameter p1("PAR_1", 1.0, 1.0, 5.0, true);
  parameters.add_parameter(p1);
  autotune::limited_continuous_parameter p2("PAR_2", 1.0, 1.0, 5.0, false);
  parameters.add_parameter(p2);

  std::function<bool(int)> test_result = [](int) -> bool { return true; };

  int a = 5;

  autotune::tuners::monte_carlo tuner(autotune::run_monte_carlo_kernel,
                                      parameters, 10, 100);
  tuner.setup_test(test_result);
  tuner.set_verbose(true);
  autotune::randomizable_set optimal_parameters = tuner.tune(a);
  optimal_parameters.print_values();
  // bool check1 = optimal_parameters[0]->get_value().compare("1.000000") != 0;
  // BOOST_CHECK(check1);
  // bool check2 = optimal_parameters[1]->get_value().compare("1.000000") != 0;
  // BOOST_CHECK(check2);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(generalized_kernel)

BOOST_AUTO_TEST_CASE(run_generalized_kernel) {
  std::string in_kernel_parameter_1;
  std::string in_kernel_parameter_2;

  autotune::generalized_test_kernel.set_kernel_functor(
      [&in_kernel_parameter_1, &in_kernel_parameter_2](double a) -> double {
        std::cout << "in_kernel_parameter_1: " << in_kernel_parameter_1
                  << std::endl;
        std::cout << "in_kernel_parameter_2: " << in_kernel_parameter_2
                  << std::endl;
        if (in_kernel_parameter_1.compare("\"zwei\"") == 0 &&
            in_kernel_parameter_2.compare("3") == 0) {
          std::cout << "fast kernel" << std::endl;
        } else if (in_kernel_parameter_1.compare("\"zwei\"") == 0 ||
                   in_kernel_parameter_2.compare("3") == 0) {
          std::this_thread::sleep_for(std::chrono::milliseconds(25));
        } else {
          std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        return a + 3;
      });
  autotune::generalized_test_kernel.set_create_parameter_file_functor(
      [&in_kernel_parameter_1, &in_kernel_parameter_2](
          autotune::parameter_value_set &parameter_values) {
        in_kernel_parameter_1 = parameter_values["PAR_1"];
        in_kernel_parameter_2 = parameter_values["PAR_2"];
      });
  autotune::generalized_test_kernel.set_verbose(true);

  autotune::countable_set parameters;
  autotune::fixed_set_parameter<std::string> p1("PAR_1",
                                                {"eins", "zwei", "drei"});
  parameters.add_parameter(p1);
  autotune::countable_continuous_parameter p2("PAR_2", 1.0, 1.0, 1.0, 5.0);
  parameters.add_parameter(p2);

  BOOST_CHECK(autotune::generalized_test_kernel(3) == 6);

  autotune::tuners::bruteforce tuner(autotune::generalized_test_kernel,
                                     parameters);

  std::function<bool(double)> test_result = [](double) -> bool { return true; };
  tuner.setup_test(test_result);
  tuner.set_verbose(true);
  double a = 3;
  autotune::countable_set optimal_parameters = tuner.tune(a);

  // bool check1 = optimal_parameters[0]->get_value().compare("\"eins\"") == 0;
  // BOOST_CHECK(check1);
  // bool check2 = optimal_parameters[1]->get_value().compare("2.000000") == 0;
  // BOOST_CHECK(check2);
}

BOOST_AUTO_TEST_SUITE_END()
