#pragma once

#include <array>
#include <cstddef>
#include <iostream>

#include "expression.hpp"

namespace opttmp {
namespace vectorization {

// type holding the actual data and triggering evaluation of expressions
template <typename vc_type, size_t elements>
class register_array
    : public expression<register_array<vc_type, elements>, vc_type, elements> {
  std::array<vc_type, elements> elems;

public:
  const vc_type &operator[](size_t i) const { return elems[i]; }
  vc_type &operator[](size_t i) { return elems[i]; }
  vc_type &set(size_t i) { return elems[i]; }
  size_t size() const { return elements; }

  // converting constructor (should not need a operator=)
  template <typename right_specialized>
  register_array(const expression<right_specialized, vc_type, elements> &e) {
    for (size_t i = 0; i < elements; i++) {
      elems[i] = e[i];
    }
  }

  register_array() {}
  // register_array(size_t n) : elems(n) {}

  register_array(vc_type value) {
    for (size_t i = 0; i < elements; i++) {
      elems[i] = value;
    }
  }

  template <typename vc_flag>
  register_array(typename vc_type::value_type *mem, vc_flag t) {
    for (size_t i = 0; i < elements; i++) {
      elems[i] = vc_type(mem + (i * vc_type::size()), t);
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

  // template <typename specialized_expr>
  // auto &operator=(expression<specialized_expr, vc_type, elements> &other) {
  //   for (size_t i = 0; i < elements; i++) {
  //     elems[i] = other[i];
  //   }
  //   return &this;
  // }

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
}
}
