#pragma once

#include "multiindex.hpp"

#include <vector>

namespace octotiger {
namespace fmm {

extern std::vector<multiindex<>> stencil;

void calculate_stencil();

} // namespace fmm
} // namespace octotiger
