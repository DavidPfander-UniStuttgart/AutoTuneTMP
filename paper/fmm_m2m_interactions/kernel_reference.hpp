#pragma once

#include <memory>
#include <vector>

#include "interaction_types.hpp"
#include "taylor.hpp"

namespace kernel_reference {

void compute_interactions_reference(
    std::vector<multipole> &M,
    std::vector<std::shared_ptr<std::vector<space_vector>>> &com_ptr,
    gsolve_type type,
    std::vector<neighbor_gravity_type> &all_neighbor_interaction_data,
    std::vector<expansion> &L, std::vector<space_vector> &L_c);

namespace detail {

void compute_interactions_inner(
    std::vector<multipole> &M,
    std::vector<std::shared_ptr<std::vector<space_vector>>> &com_ptr,
    gsolve_type type, std::vector<expansion> &L,
    std::vector<space_vector> &L_c);

void compute_boundary_interactions_multipole_multipole(
    geo::direction &dir, std::vector<multipole> &M,
    std::vector<std::shared_ptr<std::vector<space_vector>>> &com_ptr,
    gsolve_type type, const gravity_boundary_type &mpoles,
    std::vector<expansion> &L, std::vector<space_vector> &L_c);
}
}
