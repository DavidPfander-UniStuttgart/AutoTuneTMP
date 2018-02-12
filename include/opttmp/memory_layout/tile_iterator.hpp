/*
 * tile_iterator.hpp
 *
 *  Created on: Nov 5, 2016
 *      Author: pfandedd
 */

#pragma once

#include "tile_array.hpp"
#include "tile_view.hpp"
#include <vector>

namespace memory_layout {

namespace detail {
template <size_t dim, size_t cur_dim, typename T, typename U, typename F>
typename std::enable_if<cur_dim == dim, void>::type
iterate_tile_dim(std::vector<T, U> &tiled,
                 const tiling_configuration &tiling_info,
                 // size_t (&tile_index)[dim],
                 std::array<size_t, dim> &tile_index, F f) {

  memory_layout::tile_view<dim, T, U> v(tiled, tile_index, tiling_info);
  f(std::move(v));
}

template <size_t dim, size_t cur_dim, typename T, typename U, typename F>
typename std::enable_if<cur_dim != dim, void>::type
iterate_tile_dim(std::vector<T, U> &tiled,
                 const tiling_configuration &tiling_info,
                 // size_t (&partial_tile_index)[dim],
                 std::array<size_t, dim> &partial_tile_index, F f) {
  const tiling_info_dim &cur_info = tiling_info[cur_dim];
  size_t tiles_dim = cur_info.stride / cur_info.tile_size_dir;
  for (size_t tile_index_1d = 0; tile_index_1d < tiles_dim;
       tile_index_1d += 1) {
    partial_tile_index[cur_dim] = tile_index_1d;
    iterate_tile_dim<dim, cur_dim + 1>(tiled, tiling_info, partial_tile_index,
                                       f);
  }
}
}

template <size_t dim, typename T, typename U, typename F>
void iterate_tiles(std::vector<T, U> &tiled,
                   const tiling_configuration &tiling_info, F f) {
  // size_t tile_index[dim];
  std::array<size_t, dim> tile_index;
  detail::iterate_tile_dim<dim, 0>(tiled, tiling_info, tile_index, f);
}

///////////// templates for partial matrix iteration ///////////////
namespace detail {
template <size_t dim, size_t cur_dim, typename T, typename U, typename F>
typename std::enable_if<cur_dim == dim, void>::type iterate_tile_dim_partial(
    std::vector<T, U> &tiled, const std::array<size_t, dim> start,
    const std::array<size_t, dim> stop, const tiling_configuration &tiling_info,
    // size_t (&tile_index)[dim],
    std::array<size_t, dim> &tile_index, F f) {

  memory_layout::tile_view<dim, T, U> v(tiled, tile_index, tiling_info);
  f(std::move(v));
}

template <size_t dim, size_t cur_dim, typename T, typename U, typename F>
typename std::enable_if<cur_dim != dim, void>::type iterate_tile_dim_partial(
    std::vector<T, U> &tiled, const std::array<size_t, dim> start,
    const std::array<size_t, dim> stop, const tiling_configuration &tiling_info,
    // size_t (&partial_tile_index)[dim],
    std::array<size_t, dim> &partial_tile_index, F f) {
  const tiling_info_dim &cur_info = tiling_info[cur_dim];
  // size_t tiles_dim = cur_info.stride / cur_info.tile_size_dir;
  // for (size_t tile_index_1d = 0; tile_index_1d < tiles_dim;
  //      tile_index_1d += 1) {
  size_t start_tile_dim = start[cur_dim] / cur_info.tile_size_dir;
  size_t stop_tile_dim = stop[cur_dim] / cur_info.tile_size_dir;
  for (size_t tile_index_1d = start_tile_dim; tile_index_1d < stop_tile_dim;
       tile_index_1d += 1) {
    partial_tile_index[cur_dim] = tile_index_1d;
    iterate_tile_dim_partial<dim, cur_dim + 1>(tiled, start, stop, tiling_info,
                                               partial_tile_index, f);
  }
}
}

template <size_t dim, typename T, typename U, typename F>
void iterate_tiles_partial(std::vector<T, U> &tiled,
                           std::array<size_t, dim> start,
                           std::array<size_t, dim> stop,
                           const tiling_configuration &tiling_info, F f) {
  // size_t tile_index[dim];
  std::array<size_t, dim> tile_index;
  detail::iterate_tile_dim_partial<dim, 0>(tiled, start, stop, tiling_info,
                                           tile_index, f);
}
}
