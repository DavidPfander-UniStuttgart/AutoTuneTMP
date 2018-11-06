#include <algorithm>
#include <array>
#include <iostream>
#include <vector>

constexpr uint64_t N = 4;
constexpr uint64_t c = 3;

// classical AoS
namespace opttmp {
namespace memory_layout {
enum class store_tag { SoA, AoS };

template <class T, size_t S, size_t c> struct AoS_store {
  std::array<T, S * c> data;
  // general strategy: do the index computation in the class
  // the input is the same, independent from the format
  // T &at(size_t i, size_t comp) { return data[i][comp]; }
  T &at(size_t i, size_t comp) { return data[i * c + comp]; }
  T *ptr(size_t i, size_t comp) { return data.data() + i * c + comp; };
};

// // classical SoA
template <class T, size_t S, size_t c> struct SoA_store {
  std::array<T, S * c> data;

  // general strategy: do the index computation in the class
  // the input is the same, independent from the format
  T &at(size_t i, size_t comp) { return data[comp * S + i]; }
  T *ptr(size_t i, size_t comp) { return data.data() + comp * S + i; };
};

template <store_tag tag, class T, size_t S, size_t c>
struct switchable_store
    : public std::conditional<(tag == store_tag::SoA), SoA_store<T, S, c>,
                              AoS_store<T, S, c>>::type {
  constexpr size_t entries() { return S; }
  constexpr size_t components() { return components; }
  constexpr size_t size() { return S * components; }

  template <store_tag other_tag>
  void convert(switchable_store<other_tag, T, S, c> &other) {
    for (size_t i = 0; i < S; i += 1) {
      for (size_t comp = 0; comp < c; comp += 1) {
        this->at(i, comp) = other.at(i, comp);
      }
    }
  }
};
} // namespace memory_layout
} // namespace opttmp

using namespace opttmp::memory_layout;

int main(void) {
  // A as[5];
  // for (size_t i = 0; i < N; i += 1) {
  //   as[i].a = i;
  //   as[i].b = i + 1;
  //   as[i].c = i + 2;
  //   // std::cout << "A.a:" << std::endl;
  // }

  // for (size_t i = 0; i < N; i += 1) {
  //   std::cout << "as[" << i << "].a: " << as[i].a << " as[" << i
  //             << "].b: " << as[i].b << " as[" << i << "].c: " << as[i].c
  //             << std::endl;
  // }

  {
    std::cout << "SoA_store:" << std::endl;
    SoA_store<int, N, c> as;
    for (size_t i = 0; i < N; i += 1) {
      for (size_t comp = 0; comp < c; comp += 1) {
        as.at(i, comp) = i + comp;
      }
    }

    for (size_t i = 0; i < N; i += 1) {
      for (size_t comp = 0; comp < c; comp += 1) {
        if (comp > 0) {
          std::cout << ", ";
        }
        std::cout << "as.at(" << i << ", " << comp << ") = " << as.at(i, comp);
      }
      std::cout << std::endl;
    }
  }

  {
    std::cout << "AoS_store:" << std::endl;
    AoS_store<int, N, c> sa;
    for (size_t i = 0; i < N; i += 1) {
      for (size_t comp = 0; comp < c; comp += 1) {
        sa.at(i, comp) = i + comp;
      }
    }

    for (size_t i = 0; i < N; i += 1) {
      for (size_t comp = 0; comp < c; comp += 1) {
        if (comp > 0) {
          std::cout << ", ";
        }
        std::cout << "sa.at(" << i << ", " << comp << ") = " << sa.at(i, comp);
      }
      std::cout << std::endl;
    }
  }

  {
    std::cout << "switchable (as SoA):" << std::endl;
    switchable_store<store_tag::SoA, int, N, c> s;
    for (size_t i = 0; i < N; i += 1) {
      for (size_t comp = 0; comp < c; comp += 1) {
        s.at(i, comp) = i + comp;
      }
    }

    for (size_t i = 0; i < N; i += 1) {
      for (size_t comp = 0; comp < c; comp += 1) {
        if (comp > 0) {
          std::cout << ", ";
        }
        std::cout << "s.at(" << i << ", " << comp << ") = " << s.at(i, comp);
      }
      std::cout << std::endl;
    }
    for (size_t i = 0; i < N; i += 1) {
      for (size_t comp = 0; comp < c; comp += 1) {
        if (comp > 0) {
          std::cout << ", ";
        }
        std::cout << "s.ptr(" << i << ", " << comp << ") = " << s.ptr(i, comp);
      }
      std::cout << std::endl;
    }
  }

  {
    std::cout << "switchable (as AoS):" << std::endl;
    switchable_store<store_tag::AoS, int, N, c> s;
    for (size_t i = 0; i < N; i += 1) {
      for (size_t comp = 0; comp < c; comp += 1) {
        s.at(i, comp) = i + comp;
      }
    }

    for (size_t i = 0; i < N; i += 1) {
      for (size_t comp = 0; comp < c; comp += 1) {
        if (comp > 0) {
          std::cout << ", ";
        }
        std::cout << "s.at(" << i << ", " << comp << ") = " << s.at(i, comp);
      }
      std::cout << std::endl;
    }
    for (size_t i = 0; i < N; i += 1) {
      for (size_t comp = 0; comp < c; comp += 1) {
        if (comp > 0) {
          std::cout << ", ";
        }
        std::cout << "s.ptr(" << i << ", " << comp << ") = " << s.ptr(i, comp);
      }
      std::cout << std::endl;
    }
  }

  return 0;
}
