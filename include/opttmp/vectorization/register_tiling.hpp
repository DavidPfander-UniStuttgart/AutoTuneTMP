#pragma once

#include "expression.hpp"
#include "register_array.hpp"

#include <Vc/Vc>

namespace opttmp {
namespace vectorization {

/////////////////////////////////////////////////
// expressions representing different operations
/////////////////////////////////////////////////

// expression for sums
template <typename left_expr, typename right_expr>
class sum_expression : public expression<sum_expression<left_expr, right_expr>,
                                         typename left_expr::expr_vc_type,
                                         left_expr::expr_elements> {

  const left_expr &u;
  const right_expr &v;

public:
  sum_expression(const left_expr &u, const right_expr &v) : u(u), v(v) {}

  typename left_expr::expr_vc_type operator[](size_t i) const {
    return u[i] + v[i];
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

// expression for sub
template <typename left_expr, typename right_expr>
class sub_expression : public expression<sub_expression<left_expr, right_expr>,
                                         typename left_expr::expr_vc_type,
                                         left_expr::expr_elements> {

  left_expr const &u;
  right_expr const &v;

public:
  sub_expression(left_expr const &u, right_expr const &v) : u(u), v(v) {}

  typename left_expr::expr_vc_type operator[](size_t i) const {
    return u[i] - v[i];
  }

  size_t size() const { return left_expr::expr_elements; }
};

template <typename left_expr, typename scalar>
class sub_scalar_right_expression
    : public expression<sub_scalar_right_expression<left_expr, scalar>,
                        typename left_expr::expr_vc_type,
                        left_expr::expr_elements> {

  left_expr const &u;
  scalar const &s;

public:
  sub_scalar_right_expression(left_expr const &u, scalar const &s)
      : u(u), s(s) {}

  typename left_expr::expr_vc_type operator[](size_t i) const {
    return u[i] - s;
  }

  size_t size() const { return left_expr::expr_elements; }
};

template <typename right_expr, typename scalar>
class sub_scalar_left_expression
    : public expression<sub_scalar_left_expression<right_expr, scalar>,
                        typename right_expr::expr_vc_type,
                        right_expr::expr_elements> {

  right_expr const &u;
  scalar const &s;

public:
  sub_scalar_left_expression(scalar const &s, right_expr const &u)
      : u(u), s(s) {}

  typename right_expr::expr_vc_type operator[](size_t i) const {
    return s - u[i];
  }

  size_t size() const { return right_expr::expr_elements; }
};

// expression for multiplication
template <typename left_expr, typename right_expr>
class mult_expression
    : public expression<mult_expression<left_expr, right_expr>,
                        typename left_expr::expr_vc_type,
                        left_expr::expr_elements> {

  left_expr const &u;
  right_expr const &v;

public:
  mult_expression(left_expr const &u, right_expr const &v) : u(u), v(v) {}

  typename left_expr::expr_vc_type operator[](size_t i) const {
    return u[i] * v[i];
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

// expression for unary sub
template <typename left_expr>
class sub_unary_expression : public expression<sub_unary_expression<left_expr>,
                                               typename left_expr::expr_vc_type,
                                               left_expr::expr_elements> {

  left_expr const &u;

public:
  sub_unary_expression(left_expr const &u) : u(u) {}

  typename left_expr::expr_vc_type operator[](size_t i) const { return -u[i]; }

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

// operators for sum
template <typename left_specialized, typename right_specialized,
          typename vc_type, size_t elements>
const auto
operator+(const expression<left_specialized, vc_type, elements> &u,
          const expression<right_specialized, vc_type, elements> &v) {
  return sum_expression<expression<left_specialized, vc_type, elements>,
                        expression<right_specialized, vc_type, elements>>(u, v);
}

template <typename left_specialized, typename vc_type, size_t elements>
const auto operator+(const expression<left_specialized, vc_type, elements> &u,
                     const vc_type &s) {
  return sum_scalar_expression<expression<left_specialized, vc_type, elements>,
                               vc_type>(u, s);
}

template <typename right_specialized, typename vc_type, size_t elements>
const auto
operator+(const vc_type &s,
          const expression<right_specialized, vc_type, elements> &u) {
  // using commutativity to avoid another class
  return sum_scalar_expression<expression<right_specialized, vc_type, elements>,
                               vc_type>(u, s);
}

// operators for sub
template <typename left_specialized, typename right_specialized,
          typename vc_type, size_t elements>
const auto
operator-(const expression<left_specialized, vc_type, elements> &u,
          const expression<right_specialized, vc_type, elements> &v) {
  return sub_expression<expression<left_specialized, vc_type, elements>,
                        expression<right_specialized, vc_type, elements>>(u, v);
}

template <typename left_specialized, typename vc_type, size_t elements>
const auto operator-(const expression<left_specialized, vc_type, elements> &u,
                     const vc_type &s) {
  return sub_scalar_right_expression<
      expression<left_specialized, vc_type, elements>, vc_type>(u, s);
}

template <typename right_specialized, typename vc_type, size_t elements>
const auto
operator-(const vc_type &s,
          const expression<right_specialized, vc_type, elements> &u) {
  return sub_scalar_left_expression<
      expression<right_specialized, vc_type, elements>, vc_type>(s, u);
}

// overloaded operator for multiplication
template <typename left_specialized, typename right_specialized,
          typename vc_type, size_t elements>
const auto
operator*(const expression<left_specialized, vc_type, elements> &u,
          const expression<right_specialized, vc_type, elements> &v) {
  return mult_expression<expression<left_specialized, vc_type, elements>,
                         expression<right_specialized, vc_type, elements>>(u,
                                                                           v);
}

template <typename left_specialized, typename vc_type, size_t elements>
const auto operator*(const expression<left_specialized, vc_type, elements> &u,
                     const vc_type &s) {
  return mult_scalar_expression<expression<left_specialized, vc_type, elements>,
                                vc_type>(u, s);
}

template <typename right_specialized, typename vc_type, size_t elements>
const auto
operator*(const vc_type &s,
          const expression<right_specialized, vc_type, elements> &u) {
  // using commutativity to avoid another class
  return mult_scalar_expression<
      expression<right_specialized, vc_type, elements>, vc_type>(u, s);
}

template <typename specialized, typename vc_type, size_t elements>
const auto abs(const expression<specialized, vc_type, elements> &u) {
  register_array<vc_type, elements> result; // TODO: turn into expression
  for (size_t i = 0; i < elements; i++) {
    result[i] = Vc::abs(u[i]);
  }
  return result;
}

template <typename left_specialized, typename right_specialized,
          typename vc_type, size_t elements>
const auto max(const expression<left_specialized, vc_type, elements> &u,
               const expression<right_specialized, vc_type, elements> &v) {
  register_array<vc_type, elements> result; // TODO: turn into expression
  for (size_t i = 0; i < elements; i++) {
    result[i] = Vc::max(u[i], v[i]);
  }
  return result;
}

template <typename left_specialized, typename vc_type, size_t elements>
const auto max(const expression<left_specialized, vc_type, elements> &u,
               const vc_type &s) {
  register_array<vc_type, elements> result; // TODO: turn into expression
  for (size_t i = 0; i < elements; i++) {
    result[i] = Vc::max(u[i], s);
  }
  return result;
}

template <typename right_specialized, typename vc_type, size_t elements>
const auto max(const vc_type &s,
               const expression<right_specialized, vc_type, elements> &u) {
  register_array<vc_type, elements> result; // TODO: turn into expression
  for (size_t i = 0; i < elements; i++) {
    result[i] = Vc::max(u[i], s);
  }
  return result;
}

// operator for unary minus
// expression for unary minus
template <typename left_expr, typename vc_type, size_t elements>
const auto operator-(const expression<left_expr, vc_type, elements> &l) {
  return sub_unary_expression<expression<left_expr, vc_type, elements>>(l);
}

template <typename left_specialized, typename right_specialized,
          typename vc_type, size_t elements>
const auto
operator*=(const expression<left_specialized, vc_type, elements> &u,
           const expression<right_specialized, vc_type, elements> &v) {
  auto mult_expr = u * v;
  register_array<vc_type, elements> result;
  for (size_t i = 0; i < elements; i++) {
    result[i] = mult_expr[i];
  }
  return result;
}

template <typename right_specialized, // typename left_specialized,
          typename vc_type, size_t elements>
const auto
operator+=(register_array<vc_type, elements> &u,
           const expression<right_specialized, vc_type, elements> &v) {
  // std::cout << "inside operator" << std::endl;
  // for (size_t i = 0; i < u.size(); i++) {
  //   if (i > 0) {
  //     std::cout << ", ";
  //   }
  //   std::cout << u[i];
  // }
  // std::cout << std::endl;

  auto sum_expr = u + v;
  for (size_t i = 0; i < elements; i++) {
    u[i] = sum_expr[i];
  }
  return u;
}
}
}
