#pragma once

#include <cstddef>

namespace opttmp {
namespace vectorization {

// base type, everything is a expression
// back-reference to specialized type though argument
// curiously-recurring template pattern
template <typename specialized_expression_type, typename vc_type,
          size_t elements>
class expression {
public:
  using expr_vc_type = vc_type;
  static constexpr size_t expr_elements = elements;

  const vc_type operator[](size_t i) const {
    // specialized_expression_type &spec =
    //     static_cast<specialized_expression_type &>(*this);
    // return spec.operator[](i);
    // return static_cast<specialized_expression_type const &>(*this)[i];
    return static_cast<specialized_expression_type const &>(*this)[i];
  }

  size_t size() const { return elements; }

  // The following overload conversions to E, the template argument type;
  // e.g., for register_tiling_expression<sum_expression>, this is a conversion
  // to sum_expression.
  // operator E &() { return static_cast<E &>(*this); }
  // operator const E &() const { return static_cast<const E &>(*this); }
};
}
}
