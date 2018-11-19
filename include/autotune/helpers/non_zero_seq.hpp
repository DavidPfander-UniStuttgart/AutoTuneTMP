#pragma once

#include <cstddef>
#include <type_traits>

namespace autotune {
template <size_t... indices> struct index_pack {};

namespace detail {

// decrease "stop", append last stop value
template <size_t start, size_t stop, size_t... partial>
struct gen_non_zero_seq {
  using type =
      typename gen_non_zero_seq<start, stop - 1, stop, partial...>::type;
};

// stops when start and stop have the same value
template <size_t stop, size_t... partial>
struct gen_non_zero_seq<stop, stop, partial...> {
  using type = index_pack<stop, partial...>;
};

// empty case, always enabled
template <size_t start, size_t stop, typename E = void>
struct gen_range_nonempty {
  using type = index_pack<>;
};

// non-empty case, enabled if non-empty
template <size_t start, size_t stop>
struct gen_range_nonempty<start, stop,
                          typename std::enable_if<(start < stop), void>::type> {
  using type = typename detail::gen_non_zero_seq<start, stop>::type;
};

} // namespace detail

template <size_t start, size_t stop>
using make_non_zero_sequence =
    // typename detail::gen_non_zero_seq<start, stop>::type;
    typename detail::gen_range_nonempty<start, stop>::type;

} // namespace autotune

// template <size_t... indices>
// void print_template_args(autotune::index_pack<indices...>) {
//   (std::cout << indices, ...);
//   std::cout << std::endl;
// }

// namespace autotune {
//   namespace detail {

//     template<>
//     struct gen_non_zero_seq<> {};
    
//     // special case for empty range
//     template <size_t start>
//     struct gen_non_zero_seq<start, start - 1> {
//       using type = gen_non_zero_seq<>;
//     };
  
//     // decrease "stop", append last stop value
//     template <size_t start, size_t stop, size_t... partial>
//     struct gen_non_zero_seq {
//       using type =
// 	typename gen_non_zero_seq<start, stop - 1, stop, partial...>::type;
//     };

//     // stops when start and stop have the same value
//     template <size_t stop, size_t... partial>
//     struct gen_non_zero_seq<stop, stop, partial...> {
//       using type = gen_non_zero_seq<stop, partial...>;
//     };
  
//   }

//   template <size_t start, size_t stop>
//   using make_non_zero_sequence =
//     typename detail::gen_non_zero_seq<start, stop>::type;
// }

