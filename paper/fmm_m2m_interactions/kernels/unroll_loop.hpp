#pragma once

#include <cstddef>
#include <iostream>
#include <utility>

namespace opttmp {
namespace loops {

    namespace detail {
        template <class F, size_t from, size_t increment, std::size_t... I>
        void unroll_loop(F f, std::index_sequence<I...>) {
            (f(from + I * increment), ...);
        }
    }

    template <size_t from, size_t to, size_t increment, class F>
    void unroll_loop(F f) {
        detail::unroll_loop<F, from, increment>(
            f, std::make_index_sequence<(to - from) / increment>());
    }
}
}
