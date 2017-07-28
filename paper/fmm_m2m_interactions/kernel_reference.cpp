#include "interaction_types.hpp"
#include "taylor.hpp"
#include "types.hpp"

#include "compute_factor.hpp"
#include "compute_ilist.hpp"

constexpr int to_ab_idx_map3[3][3] = {{4, 5, 6}, {5, 7, 8}, {6, 8, 9}};

constexpr int cb_idx_map[6] = {
    4, 5, 6, 7, 8, 9,
};

constexpr int to_abc_idx_map3[3][6] = {{
                                           10, 11, 12, 13, 14, 15,
                                       },
                                       {
                                           11, 13, 14, 16, 17, 18,
                                       },
                                       {
                                           12, 14, 15, 17, 18, 19,
                                       }};

constexpr int to_abcd_idx_map3[3]
                              [10] = {{20, 21, 22, 23, 24, 25, 26, 27, 28, 29},
                                      {21, 23, 24, 26, 27, 28, 30, 31, 32, 33},
                                      {22, 24, 25, 27, 28, 29, 31, 32, 33, 34}};

constexpr int bcd_idx_map[10] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

constexpr int to_abc_idx_map6[6][3] = {{10, 11, 12}, {11, 13, 14},
                                       {12, 14, 15}, {13, 16, 17},
                                       {14, 17, 18}, {15, 18, 19}};

constexpr int ab_idx_map6[6] = {4, 5, 6, 7, 8, 9};

void compute_interactions_inner(
    std::shared_ptr<std::vector<multipole>> M_ptr,
    std::vector<std::shared_ptr<std::vector<space_vector>>> &com_ptr,
    gsolve_type type, std::vector<expansion> &L,
    std::vector<space_vector> &L_c) {
  // L stores the gravitational potential
  // L should be L, (10) in the paper
  // L_c stores the correction for angular momentum
  // L_c, (20) in the paper (Dominic)

  std::fill(std::begin(L), std::end(L), ZERO);
  if (opts.ang_con) {
    std::fill(std::begin(L_c), std::end(L_c), ZERO);
  }

  // Non-leaf nodes use taylor expansion
  // For leaf node, calculates the gravitational potential between two
  // particles

  // Non-leaf means that multipole-multipole interaction (David)
  // Fetch the interaction list for the current cell (David)
  auto &interaction_list = ilist_n;
  const size_t list_size = interaction_list.size();

  // Space vector is a vector pack (David)
  // Center of masses of each cell (com_ptr1 is the center of mass of the
  // parent
  // cell)
  std::vector<space_vector> const &com0 = *(com_ptr[0]);

  size_t interaction_first_index = 0;
  while (interaction_first_index < list_size) { // simd_len
    // std::cout << "interaction_first_index: " << interaction_first_index <<
    // std::endl;

    std::array<simd_vector, NDIM> X;
    // taylor<4, simd_vector> m1;

    auto &M = *M_ptr;

    // TODO: only uses first 10 coefficients? ask Dominic
    // TODO: ask Dominic about padding
    // load variables from the same first multipole into all vector registers
    const integer iii0 = interaction_list[interaction_first_index].first;
    space_vector const &com0iii0 = com0[iii0];
    for (integer d = 0; d < NDIM; ++d) {
      // load the 3D center of masses
      X[d] = com0iii0[d];
    }

    taylor<4, simd_vector> A0;
    std::array<simd_vector, NDIM> B0 = {
        {simd_vector(ZERO), simd_vector(ZERO), simd_vector(ZERO)}};

    // stop index of the iteration -> points to first entry in
    // interaction_list
    // where the first
    // multipole doesn't match the current multipole
    size_t inner_loop_stop =
        interaction_list[interaction_first_index].inner_loop_stop;

    // to a simd-sized step in the sublist of the interaction list where the
    // outer multipole is
    // same
    // TODO: there is a possible out-of-bounds here, needs padding
    for (size_t interaction_second_index = interaction_first_index;
         interaction_second_index < inner_loop_stop;
         interaction_second_index += simd_len) {
      // std::cout << "    -> interaction_second_index: " <<
      // interaction_second_index <<
      // std::endl;

      std::array<simd_vector, NDIM> Y;
      taylor<4, simd_vector> m0;

      // TODO: this is the one remaining expensive gather step, needed Bryces
      // gather approach
      // load variables for the second multipoles, one in each vector lane
      for (size_t i = 0;
           i != simd_len && interaction_second_index + i < inner_loop_stop;
           ++i) {
        const integer iii1 =
            interaction_list[interaction_second_index + i].second;
        space_vector const &com0iii1 = com0[iii1];
        for (integer d = 0; d < NDIM; ++d) {
          // load the 3D center of masses
          Y[d][i] = com0iii1[d];
        }

        // cell specific taylor series coefficients
        multipole const &Miii1 = M[iii1];
        for (integer j = 0; j != taylor_sizes[3]; ++j) {
          m0[j][i] = Miii1[j];
        }
      }

      // n angular momentum of the cells
      taylor<4, simd_vector> n0;

      // Initalize moments and momentum
      for (size_t i = 0;
           i != simd_len && interaction_second_index + i < inner_loop_stop;
           ++i) {
        if (type != gsolve_type::RHO) {
// TODO: Can we avoid that?
#pragma GCC ivdep
          for (integer j = taylor_sizes[2]; j != taylor_sizes[3]; ++j) {
            n0[j] = ZERO;
          }
        } else {
          // TODO: let's hope this is enter rarely
          const integer iii0 = interaction_list[interaction_first_index].first;
          const integer iii1 =
              interaction_list[interaction_second_index + i].second;
          multipole const &Miii0 = M[iii0];
          multipole const &Miii1 = M[iii1];
          // this branch computes the angular momentum correction, (20) in the
          // paper
          // divide by mass of other cell
          real const tmp1 = Miii1() / Miii0();
          // real const tmp2 = Miii0() / Miii1();
          // calculating the coefficients for formula (M are the octopole
          // moments)
          // the coefficients are calculated in (17) and (18)
          for (integer j = taylor_sizes[2]; j != taylor_sizes[3]; ++j) {
            n0[j][i] = Miii1[j] - Miii0[j] * tmp1;
          }
        }
      }

      // distance between cells in all dimensions
      std::array<simd_vector, NDIM> dX;
      for (integer d = 0; d < NDIM; ++d) {
        dX[d] = X[d] - Y[d];
      }

      // R_i in paper is the dX in the code
      // D is taylor expansion value for a given X expansion of the
      // gravitational
      // potential (multipole expansion)
      taylor<5, simd_vector> D;

      // calculates all D-values, calculate all coefficients of 1/r (not the
      // potential),
      // formula (6)-(9) and (19)
      D.set_basis(dX);

      // the following loops calculate formula (10), potential from A->B and
      // B->A
      // (basically alternating)
      A0[0] += m0[0] * D[0];
      if (type != gsolve_type::RHO) {
        for (integer i = taylor_sizes[0]; i != taylor_sizes[1]; ++i) {
          A0[0] -= m0[i] * D[i];
        }
      }
      for (integer i = taylor_sizes[1]; i != taylor_sizes[2]; ++i) {
        const auto tmp = D[i] * (factor[i] * HALF);
        A0[0] += m0[i] * tmp;
      }
      for (integer i = taylor_sizes[2]; i != taylor_sizes[3]; ++i) {
        const auto tmp = D[i] * (factor[i] * SIXTH);
        A0[0] -= m0[i] * tmp;
      }

      for (integer a = 0; a < NDIM; ++a) {
        int const *ab_idx_map = to_ab_idx_map3[a];
        int const *abc_idx_map = to_abc_idx_map3[a];

        A0(a) += m0() * D(a);
        for (integer i = 0; i != 6; ++i) {
          if (type != gsolve_type::RHO && i < 3) {
            A0(a) -= m0(a) * D[ab_idx_map[i]];
          }
          const integer cb_idx = cb_idx_map[i];
          const auto tmp1 = D[abc_idx_map[i]] * (factor[cb_idx] * HALF);
          A0(a) += m0[cb_idx] * tmp1;
        }
      }

      if (type == gsolve_type::RHO) {
        for (integer a = 0; a != NDIM; ++a) {
          int const *abcd_idx_map = to_abcd_idx_map3[a];
          for (integer i = 0; i != 10; ++i) {
            const integer bcd_idx = bcd_idx_map[i];
            const auto tmp = D[abcd_idx_map[i]] * (factor[bcd_idx] * SIXTH);
            B0[a] -= n0[bcd_idx] * tmp;
          }
        }
      }

      for (integer i = 0; i != 6; ++i) {
        int const *abc_idx_map6 = to_abc_idx_map6[i];

        integer const ab_idx = ab_idx_map6[i];
        A0[ab_idx] += m0() * D[ab_idx];
        for (integer c = 0; c < NDIM; ++c) {
          const auto &tmp = D[abc_idx_map6[c]];
          A0[ab_idx] -= m0(c) * tmp;
        }
      }

      for (integer i = taylor_sizes[2]; i != taylor_sizes[3]; ++i) {
        const auto &tmp = D[i];
        A0[i] += m0[0] * tmp;
      }
    }

    multipole &Liii0 = L[iii0];

    // now add up the simd lanes
    for (integer j = 0; j != taylor_sizes[3]; ++j) {
#if Vc_IS_VERSION_2 == 0
      Liii0[j] += A0[j].sum();
#else
      Liii0[j] += Vc::reduce(A0[j]);
#endif
    }
    if (type == gsolve_type::RHO && opts.ang_con) {
      space_vector &L_ciii0 = L_c[iii0];

      for (integer j = 0; j != NDIM; ++j) {
#if Vc_IS_VERSION_2 == 0
        L_ciii0[j] += B0[j].sum();
#else
        L_ciii0[j] += Vc::reduce(B0[j]);
#endif
      }
    }

    interaction_first_index = inner_loop_stop;
  }
}

void compute_boundary_interactions_multipole_multipole(
    std::shared_ptr<std::vector<multipole>> M_ptr,
    std::vector<std::shared_ptr<std::vector<space_vector>>> &com_ptr,
    gsolve_type type, const std::vector<boundary_interaction_type> &ilist_n_bnd,
    const gravity_boundary_type &mpoles, std::vector<expansion> &L,
    std::vector<space_vector> &L_c) {
  auto &M = *M_ptr;

  // always reinitialized in innermost loop
  std::array<simd_vector, NDIM> dX;
  taylor<5, simd_vector> D;
  taylor<4, simd_vector> n0;

  std::vector<space_vector> const &com0 = *(com_ptr[0]);
  size_t list_size = ilist_n_bnd.size();
  for (size_t si = 0; si < list_size; si++) {
    std::array<simd_vector, NDIM> X;
    boundary_interaction_type const &bnd = ilist_n_bnd[si];

    for (size_t i = 0; i < simd_len; ++i) {
      const integer iii0 = bnd.first[0];
      space_vector const &com0iii0 = com0[iii0];
      for (integer d = 0; d < NDIM; ++d) {
        X[d] = com0iii0[d];
      }
    }

    taylor<4, simd_vector> A0;
    std::array<simd_vector, NDIM> B0;

    taylor<4, simd_vector> m1;
    if (type == gsolve_type::RHO) {
      const integer iii0 = bnd.first[0];

      multipole const &Miii0 = M[iii0];
      m1[0] = Miii0[0];
      for (integer j = taylor_sizes[2]; j != taylor_sizes[3]; ++j) {
        m1[j] = Miii0[j];
      }
    }

    const size_t list_size = bnd.second.size();
    // TODO: is that always divisible? -> padding?
    for (size_t li = 0; li < list_size; li += simd_len) {
      taylor<4, simd_vector> m0;
      std::array<simd_vector, NDIM> Y;

      for (size_t i = 0; i < simd_len && li + i < list_size; ++i) {
        integer index = mpoles.is_local
                            ? bnd.second[li + i]
                            : li + i; // TODO: can this line be removed?
        auto const &tmp1 = (*(mpoles.M))[index];
        for (int j = 0; j != 20; ++j) {
          m0[j][i] = tmp1[j];
        }

        auto const &tmp2 = (*(mpoles.x))[index];
        for (integer d = 0; d != NDIM; ++d) {
          Y[d][i] = tmp2[d];
        }
      }

      if (type == gsolve_type::RHO) {
        simd_vector tmp = m0[0] / m1[0];
        for (integer j = taylor_sizes[2]; j != taylor_sizes[3]; ++j) {
          n0[j] = m0[j] - m1[j] * tmp;
        }
      } else {
        // reset used range
        for (integer j = taylor_sizes[2]; j != taylor_sizes[3]; ++j) {
          n0[j] = 0.0;
        }
      }

      for (integer d = 0; d < NDIM; ++d) {
        dX[d] = X[d] - Y[d];
      }

      D.set_basis(dX);
      A0[0] += m0[0] * D[0];
      if (type != gsolve_type::RHO) {
        for (integer i = taylor_sizes[0]; i != taylor_sizes[1]; ++i) {
          A0[0] -= m0[i] * D[i];
        }
      }
      for (integer i = taylor_sizes[1]; i != taylor_sizes[2]; ++i) {
        A0[0] += m0[i] * D[i] * (factor[i] * HALF);
      }
      for (integer i = taylor_sizes[2]; i != taylor_sizes[3]; ++i) {
        A0[0] -= m0[i] * D[i] * (factor[i] * SIXTH);
      }

      for (integer a = 0; a < NDIM; ++a) {
        int const *ab_idx_map = to_ab_idx_map3[a];
        int const *abc_idx_map = to_abc_idx_map3[a];

        A0(a) += m0() * D(a);
        for (integer i = 0; i != 6; ++i) {
          if (type != gsolve_type::RHO && i < 3) {
            A0(a) -= m0(a) * D[ab_idx_map[i]];
          }
          const integer cb_idx = cb_idx_map[i];
          const auto tmp = D[abc_idx_map[i]] * (factor[cb_idx] * HALF);
          A0(a) += m0[cb_idx] * tmp;
        }
      }

      if (type == gsolve_type::RHO) {
        for (integer a = 0; a < NDIM; ++a) {
          int const *abcd_idx_map = to_abcd_idx_map3[a];
          for (integer i = 0; i != 10; ++i) {
            const integer bcd_idx = bcd_idx_map[i];
            const auto tmp = D[abcd_idx_map[i]] * (factor[bcd_idx] * SIXTH);
            B0[a] -= n0[bcd_idx] * tmp;
          }
        }
      }

      for (integer i = 0; i != 6; ++i) {
        int const *abc_idx_map6 = to_abc_idx_map6[i];

        integer const ab_idx = ab_idx_map6[i];
        A0[ab_idx] += m0() * D[ab_idx];
        for (integer c = 0; c < NDIM; ++c) {
          A0[ab_idx] -= m0(c) * D[abc_idx_map6[c]];
        }
      }

      for (integer i = taylor_sizes[2]; i != taylor_sizes[3]; ++i) {
        A0[i] += m0[0] * D[i];
      }
    }

    const integer iii0 = bnd.first[0];
    expansion &Liii0 = L[iii0];

    for (integer j = 0; j != taylor_sizes[3]; ++j) {
#if Vc_IS_VERSION_2 == 0
      Liii0[j] += A0[j].sum();
#else
      Liii0[j] += Vc::reduce(A0[j]);
#endif
    }

    if (type == gsolve_type::RHO && opts.ang_con) {
      space_vector &L_ciii0 = L_c[iii0];
      for (integer j = 0; j != NDIM; ++j) {
#if Vc_IS_VERSION_2 == 0
        L_ciii0[j] += B0[j].sum();
#else
        L_ciii0[j] += Vc::reduce(B0[j]);
#endif
      }
    }
  }
}
