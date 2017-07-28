#pragma once

void compute_interactions_inner(
    std::shared_ptr<std::vector<multipole>> M_ptr,
    std::vector<std::shared_ptr<std::vector<space_vector>>> &com_ptr,
    gsolve_type type);

void compute_boundary_interactions_multipole_multipole(
    std::shared_ptr<std::vector<multipole>> M_ptr,
    std::vector<std::shared_ptr<std::vector<space_vector>>> &com_ptr,
    gsolve_type type, const std::vector<boundary_interaction_type> &ilist_n_bnd,
    const gravity_boundary_type &mpoles);
