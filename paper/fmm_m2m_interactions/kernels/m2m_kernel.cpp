#include "m2m_kernel.hpp"

#include "../interaction_types.hpp"
#include "../types.hpp"
#include "struct_of_array_taylor.hpp"

#include <array>
#include <functional>

namespace octotiger {
namespace fmm {

m2m_kernel::m2m_kernel(std::vector<bool> &neighbor_empty, gsolve_type type)
    : neighbor_empty(neighbor_empty), type(type),
      theta_rec_squared(sqr(1.0 / opts.theta)) {
  for (size_t i = 0; i < m2m_int_vector::size(); i++) {
    offset_vector[i] = i;
  }
  vectors_check_empty();
}

void m2m_kernel::apply_stencil(
    struct_of_array_data<expansion, real, 20, ENTRIES, SOA_PADDING>
        &local_expansions_SoA,
    struct_of_array_data<space_vector, real, 3, ENTRIES, SOA_PADDING>
        &center_of_masses_SoA,
    struct_of_array_data<expansion, real, 20, ENTRIES, SOA_PADDING>
        &potential_expansions_SoA,
    struct_of_array_data<space_vector, real, 3, ENTRIES, SOA_PADDING>
        &angular_corrections_SoA,
    std::vector<multiindex<>> &stencil) {
  // for (multiindex<>& stencil_element : stencil) {
  for (size_t outer_stencil_index = 0; outer_stencil_index < stencil.size();
       outer_stencil_index += STENCIL_BLOCKING) {
    for (size_t i0 = 0; i0 < INNER_CELLS_PER_DIRECTION; i0++) {
      for (size_t i1 = 0; i1 < INNER_CELLS_PER_DIRECTION; i1++) {
        for (size_t i2 = 0; i2 < INNER_CELLS_PER_DIRECTION;
             i2 += m2m_vector::size()) {
          const multiindex<> cell_index(i0 + INNER_CELLS_PADDING_DEPTH,
                                        i1 + INNER_CELLS_PADDING_DEPTH,
                                        i2 + INNER_CELLS_PADDING_DEPTH);
          // BUG: indexing has to be done with uint32_t because of Vc limitation
          const int64_t cell_flat_index = to_flat_index_padded(cell_index);
          const multiindex<> cell_index_unpadded(i0, i1, i2);
          const int64_t cell_flat_index_unpadded =
              to_inner_flat_index_not_padded(cell_index_unpadded);

          // indices on coarser level (for outer stencil boundary)
          // implicitly broadcasts to vector
          multiindex<m2m_int_vector> cell_index_coarse(cell_index);
          for (size_t j = 0; j < m2m_int_vector::size(); j++) {
            cell_index_coarse.z[j] += j;
          }
          // note that this is the same for groups of 2x2x2 elements
          // -> maps to the same for some SIMD lanes
          cell_index_coarse.transform_coarse();

          if (type == gsolve_type::RHO) {
            this->blocked_interaction_rho(
                local_expansions_SoA, center_of_masses_SoA,
                potential_expansions_SoA, angular_corrections_SoA,

                cell_index, cell_flat_index, cell_index_coarse,
                cell_index_unpadded, cell_flat_index_unpadded, stencil,
                outer_stencil_index);
          } else {
            this->blocked_interaction_non_rho(
                local_expansions_SoA, center_of_masses_SoA,
                potential_expansions_SoA, angular_corrections_SoA, cell_index,
                cell_flat_index, cell_index_coarse, cell_index_unpadded,
                cell_flat_index_unpadded, stencil, outer_stencil_index);
          }
        }
      }
    }
  }
}

void m2m_kernel::vectors_check_empty() {
  vector_is_empty =
      std::vector<bool>(PADDED_STRIDE * PADDED_STRIDE * PADDED_STRIDE);
  for (size_t i0 = 0; i0 < PADDED_STRIDE; i0 += 1) {
    for (size_t i1 = 0; i1 < PADDED_STRIDE; i1 += 1) {
      for (size_t i2 = 0; i2 < PADDED_STRIDE; i2 += 1) {
        const multiindex<> cell_index(i0, i1, i2);
        const int64_t cell_flat_index = to_flat_index_padded(cell_index);

        const multiindex<> in_boundary_start(
            (cell_index.x / INNER_CELLS_PER_DIRECTION) - 1,
            (cell_index.y / INNER_CELLS_PER_DIRECTION) - 1,
            (cell_index.z / INNER_CELLS_PER_DIRECTION) - 1);

        const multiindex<> in_boundary_end(
            in_boundary_start.x, in_boundary_start.y,
            ((cell_index.z + m2m_int_vector::size()) /
             INNER_CELLS_PER_DIRECTION) -
                1);

        geo::direction dir_start;
        dir_start.set(in_boundary_start.x, in_boundary_start.y,
                      in_boundary_start.z);
        geo::direction dir_end;
        dir_end.set(in_boundary_end.x, in_boundary_end.y, in_boundary_end.z);

        if (neighbor_empty[dir_start.flat_index_with_center()] &&
            neighbor_empty[dir_end.flat_index_with_center()]) {
          vector_is_empty[cell_flat_index] = true;
        } else {
          vector_is_empty[cell_flat_index] = false;
        }
      }
    }
  }
}

} // namespace fmm
} // namespace octotiger
