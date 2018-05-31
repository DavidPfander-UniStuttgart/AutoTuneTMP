#pragma once

#include <cstddef>

namespace autotune {
namespace detail {
template <size_t stop, size_t start, size_t... partial>
struct gen_non_zero_seq {
  using type =
      typename gen_non_zero_seq<stop, start - 1, start, partial...>::type;
};

template <size_t stop, size_t... partial>
struct gen_non_zero_seq<stop, stop, partial...> {
  using type = gen_non_zero_seq<stop, partial...>;
};
}

template <size_t start, size_t stop>
using make_non_zero_sequence =
    typename detail::gen_non_zero_seq<start, stop>::type;
}

