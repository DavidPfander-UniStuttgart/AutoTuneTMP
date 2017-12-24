#include "opttmp/vectorization/vector_tiling.hpp"

#include <cassert>

#include <Vc/Vc>
using Vc::double_v;

// base type, everything is a expression
// back-reference to specialized type though argument
// curiously-recurring template pattern
template <typename specialized_expression_type, typename vc_type,
          size_t elements>
class expression {
public:
  using expr_vc_type = vc_type;
  static constexpr size_t expr_elements = elements;

  vc_type operator[](size_t i) const {
    return static_cast<specialized_expression_type const &>(*this)[i];
  }

  size_t size() const { return elements; }

  // The following overload conversions to E, the template argument type;
  // e.g., for register_tiling_expression<sum_expression>, this is a conversion
  // to sum_expression.
  // operator E &() { return static_cast<E &>(*this); }
  // operator const E &() const { return static_cast<const E &>(*this); }
};

// type holding the actual data and triggering evaluation of expressions
template <typename vc_type, size_t elements>
class register_array
    : public expression<register_array<vc_type, elements>, vc_type, elements> {
  std::array<vc_type, elements> elems;

public:
  vc_type operator[](size_t i) const { return elems[i]; }
  vc_type &operator[](size_t i) { return elems[i]; }
  size_t size() const { return elements; }

  register_array(size_t n) : elems(n) {}

  template <typename vc_flag>
  register_array(typename vc_type::value_type *mem, vc_flag t) {
    for (size_t i = 0; i < elements; i++) {
      elems[i] = double_v(mem + (i * vc_type::size()), t);
    }
  }

  template <typename vc_flag>
  void memstore(typename vc_type::value_type *mem, vc_flag t) {
    for (size_t i = 0; i < elements; i++) { // additional integer work
      elems[i].memstore(mem + (i * vc_type::size()), t);
    }
  }

  // register_array(std::initializer_list<vc_type> init) {
  //   for (auto i : init)
  //     elems.push_back(i);
  // }

  template <typename E>
  register_array(expression<E, vc_type, elements> const &vec) {
    for (size_t i = 0; i != vec.size(); ++i) {
      elems[i] = vec[i];
    }
  }

  void print(const std::string &name) {
    for (size_t i = 0; i < elements; i++) {
      if (i > 0) {
        std::cout << ", ";
      }
      std::cout << name << "[" << i << "] = " << elems[i];
    }
    std::cout << std::endl;
  }
};

/////////////////////////////////////////////////
// expressions representing different operations
/////////////////////////////////////////////////

// expression for sums
template <typename left_expr, typename right_expr>
class sum_expression : public expression<sum_expression<left_expr, right_expr>,
                                         typename left_expr::expr_vc_type,
                                         left_expr::expr_elements> {

  left_expr const &_u;
  right_expr const &_v;

public:
  sum_expression(left_expr const &u, right_expr const &v) : _u(u), _v(v) {}

  typename left_expr::expr_vc_type operator[](size_t i) const {
    return _u[i] + _v[i];
  }

  size_t size() const { return left_expr::expr_elements; }
};

template <typename left_expr, typename scalar>
class sum_scalar_expression
    : public expression<sum_scalar_expression<left_expr, scalar>,
                        typename left_expr::expr_vc_type,
                        left_expr::expr_elements> {

  left_expr const &u;
  scalar const &s;

public:
  sum_scalar_expression(left_expr const &u, scalar const &s) : u(u), s(s) {}

  typename left_expr::expr_vc_type operator[](size_t i) const {
    return u[i] + s;
  }

  size_t size() const { return left_expr::expr_elements; }
};

// expression for multiplication
template <typename left_expr, typename right_expr>
class mult_expression
    : public expression<mult_expression<left_expr, right_expr>,
                        typename left_expr::expr_vc_type,
                        left_expr::expr_elements> {

  left_expr const &_u;
  right_expr const &_v;

public:
  mult_expression(left_expr const &u, right_expr const &v) : _u(u), _v(v) {}

  typename left_expr::expr_vc_type operator[](size_t i) const {
    return _u[i] * _v[i];
  }

  size_t size() const { return left_expr::expr_elements; }
};

template <typename left_expr, typename scalar>
class mult_scalar_expression
    : public expression<mult_expression<left_expr, scalar>,
                        typename left_expr::expr_vc_type,
                        left_expr::expr_elements> {

  left_expr const &u;
  scalar const &s;

public:
  mult_scalar_expression(left_expr const &u, scalar const &s) : u(u), s(s) {}

  typename left_expr::expr_vc_type operator[](size_t i) const {
    return u[i] * s;
  }

  size_t size() const { return left_expr::expr_elements; }
};

/////////////////////////////////
// operators for syntactic sugar
/////////////////////////////////

// cannot work, because different types of expressions are valid and data
// classes as well
// typename std::enable_if<std::is_same<left_expr, right_expr>::value,
//                         sum_expression<left_expr, right_expr>>::type

// // overloaded operator for addition
// template <typename left_expr, typename right_expr>
// const left_expr operator+(left_expr const &u, right_expr const &v) {
//   return sum_expression<left_expr, right_expr>(u, v);
// }

// overloaded operator for addition
// template <typename left_expr, typename right_expr>
template <typename left_specialized, typename right_specialized,
          typename vc_type, size_t elements>
const auto
operator+(expression<left_specialized, vc_type, elements> const &u,
          expression<right_specialized, vc_type, elements> const &v) {
  return sum_expression<expression<left_specialized, vc_type, elements>,
                        expression<right_specialized, vc_type, elements>>(u, v);
}

template <typename left_specialized, typename vc_type, size_t elements>
const auto operator+(expression<left_specialized, vc_type, elements> const &u,
                     vc_type const &s) {
  return sum_scalar_expression<expression<left_specialized, vc_type, elements>,
                               vc_type>(u, s);
}

template <typename right_specialized, typename vc_type, size_t elements>
const auto
operator+(vc_type const &s,
          expression<right_specialized, vc_type, elements> const &u) {
  // using commutativity to avoid another class
  return sum_scalar_expression<expression<right_specialized, vc_type, elements>,
                               vc_type>(u, s);
}

// overloaded operator for multiplication
template <typename left_specialized, typename right_specialized,
          typename vc_type, size_t elements>
const auto
operator*(expression<left_specialized, vc_type, elements> const &u,
          expression<right_specialized, vc_type, elements> const &v) {
  return mult_expression<expression<left_specialized, vc_type, elements>,
                         expression<right_specialized, vc_type, elements>>(u,
                                                                           v);
}

template <typename left_specialized, typename vc_type, size_t elements>
const auto operator*(expression<left_specialized, vc_type, elements> const &u,
                     vc_type const &s) {
  return mult_scalar_expression<expression<left_specialized, vc_type, elements>,
                                vc_type>(u, s);
}

template <typename right_specialized, typename vc_type, size_t elements>
const auto
operator*(vc_type const &s,
          expression<right_specialized, vc_type, elements> const &u) {
  // using commutativity to avoid another class
  return mult_scalar_expression<
      expression<right_specialized, vc_type, elements>, vc_type>(u, s);
}

// template <typename left_expr, typename right_expr>
// mult_expression<left_expr, right_expr> const operator*(left_expr const &u,
//                                                        right_expr const &v) {
//   return mult_expression<left_expr, right_expr>(u, v);
// }

int main(void) {
  constexpr size_t data_blocking = 4;

  opttmp::vectorization::vector_array<double_v, data_blocking> result_temps_arr(
      0.0);

  std::vector<double> data1 = {1.0, 2.0, 3.0, 4.0};
  std::vector<double> data2 = {1.5, 2.5, 3.5, 4.5};
  std::vector<double> data3 = {1.75, 2.75, 3.75, 4.75};

  register_array<double_v, 2> v0(data1.data(), Vc::flags::element_aligned);
  register_array<double_v, 2> v1(data2.data(), Vc::flags::element_aligned);
  register_array<double_v, 2> v2(data3.data(), Vc::flags::element_aligned);
  double_v shared_scalar = 2.0;

  v0.print("v0");
  v1.print("v1");

  register_array<double_v, 2> addition_result = v0 + v1;

  addition_result.print("addition_result");

  // register_array<double_v, 1> v0 = {23.4, 12.5, 144.56, 90.56};

  // register_array<double_v, 1> v1 = {67.12, 34.8, 90.34, 89.30};

  // register_array<double_v, 1> v2 = {34.90, 111.9, 45.12, 90.5};

  // double_v v3 = 3.0;

  // register_array<double_v, 1> result_reference(v0.size());
  std::vector<double> result_reference(data1.size());
  for (size_t i = 0; i < result_reference.size(); i++) {
    // result_reference[i] = v0[i] + ((v1[i] * v2[i]) * v3);
    result_reference[i] = data1[i] + (data2[i] * data3[i]);
  }

  // // register_array sum = v0 + ((v1 * v2) * v3);
  register_array<double_v, 2> sum = v0 + (v1 * v2);

  sum.print("sum");

  for (size_t i = 0; i < sum.size(); i++) {
    for (size_t j = 0; j < double_v::size(); j++) {
      // std::cout << "sum[" << i << "] = " << sum[i] << std::endl;
      assert(result_reference[i * double_v::size() + j] == sum[i][j]);
    }
  }
  std::cout << "assertion checked out" << std::endl;

  std::cout << "adding scalars" << std::endl;
  register_array<double_v, 2> right_scalar = v0 + shared_scalar;
  for (size_t i = 0; i < right_scalar.size(); i++) {
    for (size_t j = 0; j < double_v::size(); j++) {
      assert(right_scalar[i][j] == v0[i][j] + shared_scalar[j]);
    }
  }
  register_array<double_v, 2> left_scalar = shared_scalar + v0;
  for (size_t i = 0; i < left_scalar.size(); i++) {
    for (size_t j = 0; j < double_v::size(); j++) {
      assert(left_scalar[i][j] == v0[i][j] + shared_scalar[j]);
    }
  }

  std::cout << "multiplying scalars" << std::endl;
  register_array<double_v, 2> right_scalar_mult = v0 * shared_scalar;
  // right_scalar_mult.print("right_scalar_mult");
  for (size_t i = 0; i < right_scalar_mult.size(); i++) {
    for (size_t j = 0; j < double_v::size(); j++) {
      assert(right_scalar_mult[i][j] == v0[i][j] * shared_scalar[j]);
    }
  }
  register_array<double_v, 2> left_scalar_mult = shared_scalar * v0;

  for (size_t i = 0; i < left_scalar_mult.size(); i++) {
    for (size_t j = 0; j < double_v::size(); j++) {
      assert(left_scalar_mult[i][j] == v0[i][j] * shared_scalar[j]);
    }
  }
}
