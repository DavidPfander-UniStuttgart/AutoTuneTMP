// included by taylor.hpp

template <>
inline void taylor<5, simd_vector>::set_basis(const std::array<simd_vector, NDIM>& X) {
    constexpr integer N = 5;
    using T = simd_vector;
    // PROF_BEGIN;

    // also highly optimized

    // A is D in the paper in formula (6)
    taylor<N, T>& A = *this;

    const T r2 = sqr(X[0]) + sqr(X[1]) + sqr(X[2]);
    T r2inv = 0.0;
    for (volatile integer i = 0; i != simd_len; ++i) {
        if (r2[i] > 0.0) {
            r2inv[i] = ONE / std::max(r2[i], 1.0e-20);
        }
    }

    // parts of formula (6)
    const T d0 = -sqrt(r2inv);
    // parts of formula (7)
    const T d1 = -d0 * r2inv;
    // parts of formula (8)
    const T d2 = T(-3) * d1 * r2inv;
    // parts of  formula (9)
    const T d3 = T(-5) * d2 * r2inv;
    //     const T d4 = -T(7) * d3 * r2inv;

    // formula (6)
    A[0] = d0;

    A[1] = X[0] * d1;
    A[2] = X[1] * d1;
    A[3] = X[2] * d1;

    T X_00 = X[0] * X[0];
    T X_11 = X[1] * X[1];
    T X_22 = X[2] * X[2];

    T X_12 = X[1] * X[2];
    T X_01 = X[0] * X[1];
    T X_02 = X[0] * X[2];

    A[4] = d2 * X_00;
    A[4] += d1;
    A[5] = d2 * X_01;
    A[6] = d2 * X_02;

    A[7] = d2 * X_11;
    A[7] += d1;
    A[8] = d2 * X_12;

    A[9] = d2 * X_22;
    A[9] += d1;

    A[10] = d3 * X_00 * X[0];
    T d2_X0 = d2 * X[0];
    // A[10] += d2 * X[0];
    // A[10] += d2 * X[0];
    // A[10] += d2 * X[0];
    A[10] += 3.0 * d2_X0;
    A[11] = d3 * X_00 * X[1];
    A[11] += d2 * X[1];
    A[12] = d3 * X_00 * X[2];
    A[12] += d2 * X[2];

    A[13] = d3 * X[0] * X_11;
    A[13] += d2 * X[0];
    A[14] = d3 * X[0] * X_12;

    A[15] = d3 * X[0] * X_22;
    // A[15] += d2 * X[0];
    A[15] += d2_X0;

    A[16] = d3 * X_11 * X[1];
    T d2_X1 = d2 * X[1];
    // A[16] += d2 * X[1];
    // A[16] += d2 * X[1];
    // A[16] += d2 * X[1];
    A[16] += 3.0 * d2_X1;

    A[17] = d3 * X_11 * X[2];
    A[17] += d2 * X[2];

    A[18] = d3 * X[1] * X_22;
    A[18] += d2 * X[1];

    A[19] = d3 * X_22 * X[2];
    T d2_X2 = d2 * X[2];
    // A[19] += d2 * X[2];
    // A[19] += d2 * X[2];
    // A[19] += d2 * X[2];
    A[19] += 3.0 * d2_X2;

    A[20] = sqr(X[0]) * d3 + 2.0 * d2;
    T d3_X00 = d3 * X_00;
    // A[20] += d3 * X_00;
    // A[20] += d3 * X_00;
    A[20] += d2;
    // A[20] += d3 * X_00;
    // A[20] += d3 * X_00;
    // A[20] += d3 * X_00;
    A[20] += 5.0 * d3_X00;
    T d3_X01 = d3 * X_01;
    // A[21] = d3 * X_01;
    // A[21] += d3 * X_01;
    // A[21] += d3 * X_01;
    A[21] = 3.0 * d3_X01;
    T d3_X02 = d3 * X_02;
    // A[22] = d3 * X_02;
    // A[22] += d3 * X_02;
    // A[22] += d3 * X_02;
    A[22] = 3.0 * d3_X02;
    A[23] = d2;
    T d3_X11 = d3 * X_11;
    // A[23] += d3 * X_11;
    A[23] += d3_X11;
    A[23] += d3 * X_00;
    T d3_X12 = d3 * X_12;
    // A[24] = d3 * X_12;
    A[24] = d3_X12;
    A[25] = d2;
    T d3_X22 = d3 * X_22;
    // A[25] += d3 * X_22;
    A[25] += d3_X22;
    // A[25] += d3 * X_00;
    A[25] += d3_X00;
    // A[26] = d3 * X_01;
    // A[26] += d3 * X_01;
    // A[26] += d3 * X_01;
    A[26] = 3.0 * d3_X01;
    A[27] = d3 * X_02;
    A[28] = d3 * X_01;
    // A[29] = d3 * X_02;
    // A[29] += d3 * X_02;
    // A[29] += d3 * X_02;
    A[29] = 3.0 * d3_X02;
    A[30] = sqr(X[1]) * d3 + 2.0 * d2;
    // A[30] += d3 * X_11;
    // A[30] += d3 * X_11;
    A[30] += d2;
    // A[30] += d3 * X_11;
    // A[30] += d3 * X_11;
    // A[30] += d3 * X_11;
    A[30] += 5.0 * d3_X11;

    // A[31] = d3 * X_12;
    // A[31] += d3 * X_12;
    // A[31] += d3 * X_12;
    A[31] = 3.0 * d3_X12;
    A[32] = d2;
    // A[32] += d3 * X_22;
    A[32] += d3_X22;
    // A[32] += d3 * X_11;
    A[32] += d3_X11;
    // A[33] = d3 * X_12;
    // A[33] += d3 * X_12;
    // A[33] += d3 * X_12;
    A[33] = 3.0 * d3_X12;
    A[34] = sqr(X[2]) * d3 + 2.0 * d2;
    // A[34] += d3 * X_22;
    // A[34] += d3 * X_22;
    A[34] += d2;
    // A[34] += d3 * X_22;
    // A[34] += d3 * X_22;
    // A[34] += d3 * X_22;
    A[34] += 5.0 * d3_X22;

    // PROF_END;
}

template <>
inline void taylor<5, real>::set_basis(const std::array<real, NDIM>& X) {
    constexpr integer N = 5;
    using T = real;
    // PROF_BEGIN;

    // also highly optimized

    // A is D in the paper in formula (6)
    taylor<N, T>& A = *this;

    const T r2 = sqr(X[0]) + sqr(X[1]) + sqr(X[2]);
    T r2inv = 0.0;
    if (r2 > 0.0) {
        r2inv = ONE / std::max(r2, 1.0e-20);
    }

    // parts of formula (6)
    const T d0 = -sqrt(r2inv);
    // parts of formula (7)
    const T d1 = -d0 * r2inv;
    // parts of formula (8)
    const T d2 = T(-3) * d1 * r2inv;
    // parts of  formula (9)
    const T d3 = T(-5) * d2 * r2inv;
    //     const T d4 = -T(7) * d3 * r2inv;

    // formula (6)
    A[0] = d0;

    A[1] = X[0] * d1;
    A[2] = X[1] * d1;
    A[3] = X[2] * d1;

    T X_00 = X[0] * X[0];
    T X_11 = X[1] * X[1];
    T X_22 = X[2] * X[2];

    T X_12 = X[1] * X[2];
    T X_01 = X[0] * X[1];
    T X_02 = X[0] * X[2];

    A[4] = d2 * X_00;
    A[4] += d1;
    A[5] = d2 * X_01;
    A[6] = d2 * X_02;

    A[7] = d2 * X_11;
    A[7] += d1;
    A[8] = d2 * X_12;

    A[9] = d2 * X_22;
    A[9] += d1;

    A[10] = d3 * X_00 * X[0];
    T d2_X0 = d2 * X[0];
    // A[10] += d2 * X[0];
    // A[10] += d2 * X[0];
    // A[10] += d2 * X[0];
    A[10] += 3.0 * d2_X0;
    A[11] = d3 * X_00 * X[1];
    A[11] += d2 * X[1];
    A[12] = d3 * X_00 * X[2];
    A[12] += d2 * X[2];

    A[13] = d3 * X[0] * X_11;
    A[13] += d2 * X[0];
    A[14] = d3 * X[0] * X_12;

    A[15] = d3 * X[0] * X_22;
    // A[15] += d2 * X[0];
    A[15] += d2_X0;

    A[16] = d3 * X_11 * X[1];
    T d2_X1 = d2 * X[1];
    // A[16] += d2 * X[1];
    // A[16] += d2 * X[1];
    // A[16] += d2 * X[1];
    A[16] += 3.0 * d2_X1;

    A[17] = d3 * X_11 * X[2];
    A[17] += d2 * X[2];

    A[18] = d3 * X[1] * X_22;
    A[18] += d2 * X[1];

    A[19] = d3 * X_22 * X[2];
    T d2_X2 = d2 * X[2];
    // A[19] += d2 * X[2];
    // A[19] += d2 * X[2];
    // A[19] += d2 * X[2];
    A[19] += 3.0 * d2_X2;

    A[20] = sqr(X[0]) * d3 + 2.0 * d2;
    T d3_X00 = d3 * X_00;
    // A[20] += d3 * X_00;
    // A[20] += d3 * X_00;
    A[20] += d2;
    // A[20] += d3 * X_00;
    // A[20] += d3 * X_00;
    // A[20] += d3 * X_00;
    A[20] += 5.0 * d3_X00;
    T d3_X01 = d3 * X_01;
    // A[21] = d3 * X_01;
    // A[21] += d3 * X_01;
    // A[21] += d3 * X_01;
    A[21] = 3.0 * d3_X01;
    T d3_X02 = d3 * X_02;
    // A[22] = d3 * X_02;
    // A[22] += d3 * X_02;
    // A[22] += d3 * X_02;
    A[22] = 3.0 * d3_X02;
    A[23] = d2;
    T d3_X11 = d3 * X_11;
    // A[23] += d3 * X_11;
    A[23] += d3_X11;
    A[23] += d3 * X_00;
    T d3_X12 = d3 * X_12;
    // A[24] = d3 * X_12;
    A[24] = d3_X12;
    A[25] = d2;
    T d3_X22 = d3 * X_22;
    // A[25] += d3 * X_22;
    A[25] += d3_X22;
    // A[25] += d3 * X_00;
    A[25] += d3_X00;
    // A[26] = d3 * X_01;
    // A[26] += d3 * X_01;
    // A[26] += d3 * X_01;
    A[26] = 3.0 * d3_X01;
    A[27] = d3 * X_02;
    A[28] = d3 * X_01;
    // A[29] = d3 * X_02;
    // A[29] += d3 * X_02;
    // A[29] += d3 * X_02;
    A[29] = 3.0 * d3_X02;
    A[30] = sqr(X[1]) * d3 + 2.0 * d2;
    // A[30] += d3 * X_11;
    // A[30] += d3 * X_11;
    A[30] += d2;
    // A[30] += d3 * X_11;
    // A[30] += d3 * X_11;
    // A[30] += d3 * X_11;
    A[30] += 5.0 * d3_X11;

    // A[31] = d3 * X_12;
    // A[31] += d3 * X_12;
    // A[31] += d3 * X_12;
    A[31] = 3.0 * d3_X12;
    A[32] = d2;
    // A[32] += d3 * X_22;
    A[32] += d3_X22;
    // A[32] += d3 * X_11;
    A[32] += d3_X11;
    // A[33] = d3 * X_12;
    // A[33] += d3 * X_12;
    // A[33] += d3 * X_12;
    A[33] = 3.0 * d3_X12;
    A[34] = sqr(X[2]) * d3 + 2.0 * d2;
    // A[34] += d3 * X_22;
    // A[34] += d3 * X_22;
    A[34] += d2;
    // A[34] += d3 * X_22;
    // A[34] += d3 * X_22;
    // A[34] += d3 * X_22;
    A[34] += 5.0 * d3_X22;

    // PROF_END;
}
