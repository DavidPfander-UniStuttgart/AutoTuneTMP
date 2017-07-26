#pragma once

#define OCTOTIGER_FORCEINLINE __attribute__((always_inline))
#define sqr(a) (a * a)
#define NDIM 3

using integer = int64_t;
using real = double;

constexpr real ZERO = 0.0;
constexpr real ONE = 1.0;
constexpr real HALF = 0.5;
constexpr real SIXTH = 1.0 / 6.0;

constexpr integer INX = 8;
constexpr integer G_NX = INX;
constexpr integer PATCH_SIZE = INX * INX * INX;

enum class gsolve_type { RHO, NON_RHO };
static struct options { double theta = 0.35; } opts;

#include <Vc/Vc>

#if defined(Vc_HAVE_AVX512F)
using simd_vector = Vc::datapar<double, Vc::datapar_abi::avx512>;
using int_simd_vector = Vc::datapar<int32_t, Vc::datapar_abi::avx512>;
using v4sd = Vc::datapar<double, Vc::datapar_abi::avx>;
constexpr std::size_t simd_len = simd_vector::size();
#else
using simd_vector =
    typename Vc::datapar<double, Vc::datapar_abi::fixed_size<8>>;
using int_simd_vector =
    typename Vc::datapar<int32_t, Vc::datapar_abi::fixed_size<8>>;
using v4sd = typename Vc::datapar<double, Vc::datapar_abi::fixed_size<4>>;
constexpr std::size_t simd_len = simd_vector::size();
#endif

using space_vector = Vc::datapar<double, Vc::datapar_abi::fixed_size<4>>;
