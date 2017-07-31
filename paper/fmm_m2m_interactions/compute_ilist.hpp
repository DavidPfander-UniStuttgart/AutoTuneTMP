#pragma once

#include <vector>

#include "interaction_types.hpp"

// non-root and non-leaf node (David)
extern std::vector<interaction_type> ilist_n;
// monopole-monopole interactions? (David)
extern std::vector<interaction_type> ilist_d;
// interactions of root node (empty list? boundary stuff?) (David)
// extern std::vector<interaction_type> ilist_r;
// extern std::vector<std::vector<boundary_interaction_type>> ilist_d_bnd;
extern std::vector<std::vector<boundary_interaction_type>> ilist_n_bnd;
// extern std::vector<std::vector<boundary_interaction_type>> ilist_n_bnd_new;

void compute_ilist();
