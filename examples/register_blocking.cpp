#include "opttmp/vectorization/register_tiling.hpp"
#include <Vc/Vc>

using Vc::double_v;

using namespace opttmp::vectorization;

void check_and_throw(bool condition) {
  if (!condition) {
    throw;
  }
}

// template <typename left_expr, typename right_expr>
// mult_expression<left_expr, right_expr> const operator*(left_expr const
// &u,
//                                                        right_expr const
//                                                        &v) {
//   return mult_expression<left_expr, right_expr>(u, v);
// }

int main(void) {
  // constexpr size_t data_blocking = 4;

  std::vector<double> negative_values_raw = {-1.5, -2.5, -3.5, -4.5,
                                             -3.0, -8.0, -3.0, -1.532};
  std::vector<double> positive_values_raw = {1.75, 2.75, 3.75, 4.75,
                                             5.75, 6.75, 7.75, 8.75};

  std::vector<double> data_up = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  std::vector<double> data_down = {7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0};
  constexpr size_t vector_elements = 8 / double_v::size();
  double_v average = 3.5;

  register_array<double_v, vector_elements> negative_values(
      negative_values_raw.data(), Vc::flags::element_aligned);
  register_array<double_v, vector_elements> positive_values(
      positive_values_raw.data(), Vc::flags::element_aligned);
  register_array<double_v, vector_elements> up(data_up.data(),
                                               Vc::flags::element_aligned);
  register_array<double_v, vector_elements> down(data_down.data(),
                                                 Vc::flags::element_aligned);
  double_v shared_scalar = -2.0;
  // register_array<double_v, vector_elements> zero = double_v(0.0);
  // register_array<double_v, vector_elements> one = double_v(1.0);
  // register_array<double_v, vector_elements> two = double_v(2.0);

  // v0.print("v0");
  // v1.print("v1");

  // auto blubb = v0 + v1;
  // std::cout << "blubb[0] = " << blubb[0] << std::endl;

  {
    // test assign operator
    register_array<double_v, vector_elements> copied = positive_values;
    for (size_t i = 0; i < vector_elements; i++) {
      auto mask = copied[i] == positive_values[i];
      check_and_throw(Vc::all_of(mask));
    }
  }

  {
    // test addition with temporary expression
    std::vector<double> reference = {0.25, 0.25,  0.25, 0.25,
                                     2.75, -1.25, 4.75, 7.218};

    auto tmp = negative_values + positive_values;
    register_array<double_v, vector_elements> addition_result = tmp;
    for (size_t i = 0; i < addition_result.size(); i++) {
      for (size_t j = 0; j < double_v::size(); j++) {

        check_and_throw(addition_result[i][j] ==
                        reference[i * double_v::size() + j]);
      }
    }
  }

  {
    // test addition without temporary expression through copy initialization
    std::vector<double> reference = {0.25, 0.25,  0.25, 0.25,
                                     2.75, -1.25, 4.75, 7.218};
    register_array<double_v, vector_elements> addition_result(negative_values +
                                                              positive_values);
    for (size_t i = 0; i < addition_result.size(); i++) {
      for (size_t j = 0; j < double_v::size(); j++) {
        check_and_throw(addition_result[i][j] ==
                        reference[i * double_v::size() + j]);
      }
    }
  }

  {
    // test addition without temporary expression through copy assignment
    std::vector<double> reference = {0.25, 0.25,  0.25, 0.25,
                                     2.75, -1.25, 4.75, 7.218};
    register_array<double_v, vector_elements> addition_result =
        negative_values + positive_values;
    for (size_t i = 0; i < addition_result.size(); i++) {
      for (size_t j = 0; j < double_v::size(); j++) {
        check_and_throw(addition_result[i][j] ==
                        reference[i * double_v::size() + j]);
      }
    }
  }

  {
    // test addition and multiplication in one expression
    std::vector<double> result_reference(data_up.size());
    for (size_t i = 0; i < result_reference.size(); i++) {
      result_reference[i] =
          data_up[i] + (negative_values_raw[i] * positive_values_raw[i]);
    }

    register_array<double_v, vector_elements> sum =
        up + (negative_values * positive_values);

    for (size_t i = 0; i < sum.size(); i++) {
      for (size_t j = 0; j < double_v::size(); j++) {
        check_and_throw(result_reference[i * double_v::size() + j] ==
                        sum[i][j]);
      }
    }
  }

  {
    // right-adding scalar
    register_array<double_v, vector_elements> right_scalar =
        negative_values + shared_scalar;
    for (size_t i = 0; i < right_scalar.size(); i++) {
      for (size_t j = 0; j < double_v::size(); j++) {
        check_and_throw(right_scalar[i][j] ==
                        negative_values[i][j] + shared_scalar[j]);
      }
    }
  }
  {
    // left-adding scalar
    register_array<double_v, vector_elements> left_scalar =
        shared_scalar + negative_values;
    for (size_t i = 0; i < left_scalar.size(); i++) {
      for (size_t j = 0; j < double_v::size(); j++) {
        check_and_throw(left_scalar[i][j] ==
                        negative_values[i][j] + shared_scalar[j]);
      }
    }
  }

  {
    // test subtraction without temporary expression through copy initialization
    std::vector<double> result_reference(data_up.size());
    for (size_t i = 0; i < result_reference.size(); i++) {
      result_reference[i] = negative_values_raw[i] - positive_values_raw[i];
    }
    register_array<double_v, vector_elements> subtraction_result(
        negative_values - positive_values);
    for (size_t i = 0; i < subtraction_result.size(); i++) {
      for (size_t j = 0; j < double_v::size(); j++) {
        check_and_throw(subtraction_result[i][j] ==
                        result_reference[i * double_v::size() + j]);
      }
    }
  }

  {
    // test subtraction without temporary expression through copy assignment
    std::vector<double> result_reference(data_up.size());
    for (size_t i = 0; i < result_reference.size(); i++) {
      result_reference[i] = negative_values_raw[i] - positive_values_raw[i];
    }
    register_array<double_v, vector_elements> subtraction_result =
        negative_values - positive_values;
    for (size_t i = 0; i < subtraction_result.size(); i++) {
      for (size_t j = 0; j < double_v::size(); j++) {
        check_and_throw(subtraction_result[i][j] ==
                        result_reference[i * double_v::size() + j]);
      }
    }
  }

  {
    // test subtraction and multiplication in one expression
    std::vector<double> result_reference(data_up.size());
    for (size_t i = 0; i < result_reference.size(); i++) {
      result_reference[i] =
          data_up[i] + (negative_values_raw[i] - positive_values_raw[i]);
    }

    register_array<double_v, vector_elements> sum =
        up + (negative_values - positive_values);

    for (size_t i = 0; i < sum.size(); i++) {
      for (size_t j = 0; j < double_v::size(); j++) {
        check_and_throw(result_reference[i * double_v::size() + j] ==
                        sum[i][j]);
      }
    }
  }

  {
    // right-subtracting scalar
    register_array<double_v, vector_elements> right_scalar =
        negative_values - shared_scalar;
    for (size_t i = 0; i < right_scalar.size(); i++) {
      for (size_t j = 0; j < double_v::size(); j++) {
        check_and_throw(right_scalar[i][j] ==
                        negative_values[i][j] - shared_scalar[j]);
      }
    }
  }
  {
    // left-subtracting scalar
    register_array<double_v, vector_elements> left_scalar =
        shared_scalar - negative_values;
    for (size_t i = 0; i < left_scalar.size(); i++) {
      for (size_t j = 0; j < double_v::size(); j++) {
        check_and_throw(left_scalar[i][j] ==
                        shared_scalar[j] - negative_values[i][j]);
      }
    }
  }

  {
    // right-multiplying scalar
    register_array<double_v, vector_elements> right_scalar_mult =
        negative_values * shared_scalar;
    for (size_t i = 0; i < right_scalar_mult.size(); i++) {
      for (size_t j = 0; j < double_v::size(); j++) {
        check_and_throw(right_scalar_mult[i][j] ==
                        negative_values[i][j] * shared_scalar[j]);
      }
    }
  }

  {
    // left-multiplying scalar
    register_array<double_v, vector_elements> left_scalar_mult =
        shared_scalar * negative_values;

    for (size_t i = 0; i < left_scalar_mult.size(); i++) {
      for (size_t j = 0; j < double_v::size(); j++) {
        check_and_throw(left_scalar_mult[i][j] ==
                        negative_values[i][j] * shared_scalar[j]);
      }
    }
    // left_scalar_mult.print("left_scalar_mult");
  }

  {
    // calculating abs of register_array
    register_array<double_v, vector_elements> abs_result = abs(negative_values);
    for (size_t i = 0; i < abs_result.size(); i++) {
      for (size_t j = 0; j < double_v::size(); j++) {
        check_and_throw(abs_result[i][j] == -negative_values[i][j]);
      }
    }
    // abs_left_scalar_mult.print("abs_left_scalar_mult");
  }

  {
    // max between to register_arrays
    std::vector<double> max_reference = {7.0, 6.0, 5.0, 4.0,
                                         4.0, 5.0, 6.0, 7.0};
    register_array<double_v, vector_elements> max_result = max(up, down);
    // max_result.print("max_result");
    for (size_t i = 0; i < max_result.size(); i++) {
      for (size_t j = 0; j < double_v::size(); j++) {
        check_and_throw(max_result[i][j] ==
                        max_reference[i * double_v::size() + j]);
      }
    }
  }

  {
    // max between to register_array and scalar (left)
    std::vector<double> max_reference = {3.5, 3.5, 3.5, 3.5,
                                         4.0, 5.0, 6.0, 7.0};
    register_array<double_v, vector_elements> max_left_result =
        max(average, up);
    // max_left_result.print("max_left_result");
    for (size_t i = 0; i < max_left_result.size(); i++) {
      for (size_t j = 0; j < double_v::size(); j++) {
        check_and_throw(max_left_result[i][j] ==
                        max_reference[i * double_v::size() + j]);
      }
    }
  }

  {
    // max between to register_array and scalar (right)
    std::vector<double> max_reference = {3.5, 3.5, 3.5, 3.5,
                                         4.0, 5.0, 6.0, 7.0};
    register_array<double_v, vector_elements> max_right_result =
        max(up, average);
    // max_right_result.print("max_right_result");
    for (size_t i = 0; i < max_right_result.size(); i++) {
      for (size_t j = 0; j < double_v::size(); j++) {
        check_and_throw(max_right_result[i][j] ==
                        max_reference[i * double_v::size() + j]);
      }
    }
  }

  {
    // operator+= on register_array
    register_array<double_v, vector_elements> result_reference = up + up;
    result_reference.print("result_reference");
    register_array<double_v, vector_elements> result(up);
    result.print("result before");
    result += up;
    result.print("result");
    for (size_t i = 0; i < result.size(); i++) {
      for (size_t j = 0; j < double_v::size(); j++) {
        // TODO: workaround for ambiguous bug in Vc
        double ref = result_reference[i][j];
        check_and_throw(result[i][j] == ref);
      }
    }
  }
}
