#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <vector>

#include "../loop/loop_nest.hpp"
#include "memory_layout_exception.hpp"
#include "tile_iterator.hpp"
#include "tiling_info_dim.hpp"
#include "util.hpp"

namespace memory_layout {

// dimension of matrix, dimension of tiles
template <size_t dim, typename T, typename U>
std::vector<T, U> make_tiled(std::vector<T, U> &org,
                             const tiling_configuration &tiling_info) {
  std::vector<T, U> tiled;
  tiled.resize(org.size());
  if (tiling_info.size() != dim) {
    throw memory_layout_exception(
        "tiling_info doesn't match specified dimension");
  }

  // min, max index for iterating within tile
  std::array<size_t, dim> min;
  for (size_t d = 0; d < dim; d++) {
    min[d] = 0;
  }
  std::array<size_t, dim> max;
  for (size_t d = 0; d < dim; d++) {
    max[d] = tiling_info[d].tile_size_dir;
  }
  // required for dynamic_loop_nest
  std::array<size_t, dim> strides;
  for (size_t d = 0; d < dim; d++) {
    strides[d] = 1;
  }

  // number of elements in dim
  std::array<size_t, dim> untiled_strides;
  for (size_t d = 0; d < dim; d++) {
    untiled_strides[d] = tiling_info[d].stride;
  }

  memory_layout::iterate_tiles<dim>(
      tiled, tiling_info,
      [&org, &untiled_strides, &tiling_info, &min, &max, &strides](auto view) {
        const std::array<size_t, dim> &tile_index = view.get_tile_index();

        std::array<size_t, dim> index_offset;
        // std::cout << "---------next tile---------" << std::endl;
        for (size_t d = 0; d < dim; d++) {
          index_offset[d] = tile_index[d] * tiling_info[d].tile_size_dir;
          // std::cout << "index_offset[" << d << "] = " << index_offset[d] <<
          // std::endl;
        }

        opttmp::loop::dynamic_loop_nest<dim>(
            min, max, strides,
            [&view, &org, &tiling_info, &index_offset,
             &untiled_strides](const std::array<size_t, dim> &inner_index) {
              std::array<size_t, dim> outer_index;
              for (size_t d = 0; d < dim; d++) {
                outer_index[d] = inner_index[d] + index_offset[d];
                // std::cout << "outer_index[" << d << "] = " << outer_index[d]
                // << std::endl;
              }
              size_t flat_outer_index =
                  flat_index(outer_index, untiled_strides);
              // std::cout << "flat_outer_index: " << flat_outer_index <<
              // std::endl;
              // std::cout << "view[index_offset] = " << view[inner_index] <<
              // std::endl;
              view[inner_index] = org[flat_outer_index];
            });

      });

  return tiled;
}

// dimension of matrix, dimension of tiles
// undo tiling inplace
template <size_t dim, typename T, typename U>
std::vector<T, U> undo_tiling(std::vector<T, U> &tiled,
                              const tiling_configuration &tiling_info) {
  std::vector<T, U> untiled(tiled.size());
  // untiled.resize(tiled.size());
  if (tiling_info.size() != dim) {
    throw memory_layout_exception(
        "tiling_info doesn't match specified dimension");
  }

  // min, max index for iterating within tile
  std::array<size_t, dim> min;
  for (size_t d = 0; d < dim; d++) {
    min[d] = 0;
  }
  std::array<size_t, dim> max;
  for (size_t d = 0; d < dim; d++) {
    max[d] = tiling_info[d].tile_size_dir;
  }
  // required for dynamic_loop_nest
  std::array<size_t, dim> strides;
  for (size_t d = 0; d < dim; d++) {
    strides[d] = 1;
  }

  // number of elements in dim
  std::array<size_t, dim> untiled_strides;
  for (size_t d = 0; d < dim; d++) {
    untiled_strides[d] = tiling_info[d].stride;
  }

  memory_layout::iterate_tiles<dim>(
      tiled, tiling_info, [&untiled, &untiled_strides, &tiling_info, &min, &max,
                           &strides](auto view) {
        const std::array<size_t, dim> &tile_index = view.get_tile_index();

        std::array<size_t, dim> index_offset;
        // std::cout << "---------next tile---------" << std::endl;
        for (size_t d = 0; d < dim; d++) {
          index_offset[d] = tile_index[d] * tiling_info[d].tile_size_dir;
          // std::cout << "index_offset[" << d << "] = " << index_offset[d]
          //           << std::endl;
        }

        opttmp::loop::dynamic_loop_nest<dim>(
            min, max, strides,
            [&view, &untiled, &tiling_info, &index_offset,
             &untiled_strides](const std::array<size_t, dim> &inner_index) {
              std::array<size_t, dim> outer_index;
              // std::cout << "untiled index ";
              for (size_t d = 0; d < dim; d++) {
                outer_index[d] = inner_index[d] + index_offset[d];
                // if (d > 0) {
                //     std::cout << ", ";
                // }
                // std::cout << outer_index[d];
              }
              size_t flat_outer_index =
                  flat_index(outer_index, untiled_strides);
              // std::cout << " flat_outer_index: " << flat_outer_index;
              // std::cout << " view[index_offset] = " << view[inner_index]
              //           << std::endl;
              untiled[flat_outer_index] = view[inner_index];
            });

      });
  return untiled;
}
}
