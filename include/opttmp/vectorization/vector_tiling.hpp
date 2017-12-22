#pragma once

#include <Vc/Vc>

namespace opttmp {
namespace vectorization {

// forward declarations for friendship
template <typename vector_type, size_t elements> class vector_array;

template <typename vector_type, size_t elements>
vector_array<vector_type, elements>
max(const vector_array<vector_type, elements> &left, const vector_type &right);
template <typename vector_type, size_t elements>
vector_array<vector_type, elements>
abs(const vector_array<vector_type, elements> &v);

// implementations
template <typename vector_type, size_t elements> class vector_array {
private:
  std::array<vector_type, elements> arr;

public:
  // do not initialize memory, because of performance
  vector_array(){};

  // converting constructor
  vector_array(typename vector_type::value_type scalar_value) {
    for (size_t i = 0; i < elements; i++) { // unrolled
      // converting constructor (broadcast)
      arr[i] = scalar_value;
    }
  }

  // converting constructor
  vector_array(vector_type value) {
    for (size_t i = 0; i < elements; i++) {
      arr[i] = value;
    }
  }

  // initialize from memory
  template <typename Vc_flag>
  vector_array(typename vector_type::value_type *mem, Vc_flag t) {
    for (size_t i = 0; i < elements; i++) {
      arr[i] = double_v(mem + (i * vector_type::size()), t);
    }
  }

  vector_array(const vector_array<vector_type, elements> &other) {
    for (size_t i = 0; i < elements; i++) {
      arr[i] = other.arr[i];
    }
  }

  // binary operators
  vector_array<vector_type, elements> operator+(const vector_type &v) {
    vector_array<vector_type, elements> r;
    for (size_t i = 0; i < elements; i++) {
      r.arr[i] = arr[i] + v;
    }
    return r;
  }
  vector_array<vector_type, elements> operator-(const vector_type &v) {
    vector_array<vector_type, elements> r;
    for (size_t i = 0; i < elements; i++) {
      r.arr[i] = arr[i] - v;
    }
    return r;
  }
  vector_array<vector_type, elements> operator*(const vector_type &v) {
    vector_array<vector_type, elements> r;
    for (size_t i = 0; i < elements; i++) {
      r.arr[i] = arr[i] * v;
    }
    return r;
  }
  vector_array<vector_type, elements> operator/(vector_type &v) {
    vector_array<vector_type, elements> r;
    for (size_t i = 0; i < elements; i++) {
      r.arr[i] = arr[i] / v;
    }
    return r;
  }

  vector_array<vector_type, elements>
  operator+(const vector_array<vector_type, elements> &other) {
    vector_array<vector_type, elements> r;
    for (size_t i = 0; i < elements; i++) {
      r.arr[i] = arr[i] + other[i];
    }
    return r;
  }
  vector_array<vector_type, elements>
  operator-(const vector_array<vector_type, elements> &other) {
    vector_array<vector_type, elements> r;
    for (size_t i = 0; i < elements; i++) {
      r.arr[i] = arr[i] - other[i];
    }
    return r;
  }
  vector_array<vector_type, elements>
  operator*(const vector_array<vector_type, elements> &other) {
    vector_array<vector_type, elements> r;
    for (size_t i = 0; i < elements; i++) {
      r.arr[i] = arr[i] * other[i];
    }
    return r;
  }
  vector_array<vector_type, elements>
  operator/(const vector_array<vector_type, elements> &other) {
    vector_array<vector_type, elements> r;
    for (size_t i = 0; i < elements; i++) {
      r.arr[i] = arr[i] / other[i];
    }
    return r;
  }
  // void operator%() { throw; }

  // unary operators
  void operator+=(const vector_type &v) {
    for (size_t i = 0; i < elements; i++) {
      arr[i] += v;
    }
  }
  void operator-=(const vector_type &v) {
    for (size_t i = 0; i < elements; i++) {
      arr[i] -= v;
    }
  }
  void operator*=(const vector_type &v) {
    for (size_t i = 0; i < elements; i++) {
      arr[i] *= v;
    }
  }
  void operator/=(const vector_type &v) {
    for (size_t i = 0; i < elements; i++) {
      arr[i] /= v;
    }
  }

  void operator+=(const vector_array<vector_type, elements> &other) {
    for (size_t i = 0; i < elements; i++) {
      arr[i] += other.arr[i];
    }
  }
  void operator-=(const vector_array<vector_type, elements> &other) {
    for (size_t i = 0; i < elements; i++) {
      arr[i] -= other.arr[i];
    }
  }
  void operator*=(const vector_array<vector_type, elements> &other) {
    for (size_t i = 0; i < elements; i++) {
      arr[i] *= other.arr[i];
    }
  }
  void operator/=(const vector_array<vector_type, elements> &other) {
    for (size_t i = 0; i < elements; i++) {
      arr[i] /= other.arr[i];
    }
  }
  // void operator%=() { throw; }

  //   vector_array<vector_type, elements>
  // operator+() {
  //   vector_array<vector_type, elements> r;
  //   for (size_t i = 0; i < elements; i++) {
  //     r.arr[i] = arr[i] + other[i];
  //   }
  //   return r;
  // }
  vector_array<vector_type, elements> operator-() {
    vector_array<vector_type, elements> r;
    for (size_t i = 0; i < elements; i++) {
      r.arr[i] = -arr[i];
    }
    return r;
  }

  template <typename Vc_flag>
  void memstore(typename vector_type::value_type *mem, Vc_flag t) {
    for (size_t i = 0; i < elements; i++) { // additional integer work
      arr[i].memstore(mem + (i * vector_type::size()), t);
    }
  }

  friend vector_array<vector_type, elements>
  opttmp::vectorization::max<vector_type, elements>(
      const vector_array<vector_type, elements> &, const vector_type &);

  friend vector_array<vector_type, elements>
  opttmp::vectorization::abs<vector_type, elements>(
      const vector_array<vector_type, elements> &);
};

template <typename vector_type, size_t elements>
vector_array<vector_type, elements>
max(const vector_array<vector_type, elements> &left, const vector_type &right) {
  vector_array<vector_type, elements> r;
  for (size_t i = 0; i < elements; i++) {
    r.arr[i] = Vc::max(left.arr[i], right);
  }
  return r;
}

template <typename vector_type, size_t elements>
vector_array<vector_type, elements>
abs(const vector_array<vector_type, elements> &v) {
  vector_array<vector_type, elements> r;
  for (size_t i = 0; i < elements; i++) {
    r.arr[i] = Vc::abs(v.arr[i]);
  }
  return r;
}
}
}
