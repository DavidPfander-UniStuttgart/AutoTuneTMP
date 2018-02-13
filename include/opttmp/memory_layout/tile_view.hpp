#pragma once

#include <vector>

#include "tiling_info_dim.hpp"

namespace memory_layout {

struct tiling_info_dim;

namespace detail {
template <size_t dim>
size_t calculate_tile_size(const tiling_configuration &tiling_info) {
  size_t tile_size = 1;
  for (size_t d = 0; d < dim; d++) {
    tile_size *= tiling_info[d].tile_size_dir;
  }
  return tile_size;
}

template <size_t dim>
size_t calculate_base_offset(const std::array<size_t, dim> &tile_index,
                             const tiling_configuration &tiling_info,
                             const size_t tile_size) {
  size_t cur_stride = 1;
  size_t base_offset = 0;
  for (size_t d = 0; d < dim; d++) {
    base_offset += tile_index[(dim - 1) - d] * cur_stride;
    cur_stride *= (tiling_info[(dim - 1) - d].stride /
                   tiling_info[(dim - 1) - d].tile_size_dir);
  }

  base_offset *= tile_size;
  return base_offset;
}

template <size_t dim>
const std::array<size_t, dim>
calculate_tiles_dir(const tiling_configuration &tiling_info) {
  std::array<size_t, dim> tiles_dir;
  for (size_t d = 0; d < dim; d++) {
    tiles_dir[d] = tiling_info[d].stride / tiling_info[d].tile_size_dir;
  }
  return tiles_dir;
}
}

template <size_t dim, typename T, typename U> class tile_view {
private:
  std::vector<T, U> &tiled;
  const size_t tile_size;
  const size_t base_offset;
  const std::array<size_t, dim> tile_index;
  const std::array<size_t, dim> tiles_dir;
  const tiling_configuration tiling_info;

public:
  tile_view(std::vector<T, U> &tiled, std::array<size_t, dim> &tile_index,
            const tiling_configuration &tiling_info)
      : tiled(tiled), tile_size(detail::calculate_tile_size<dim>(tiling_info)),
        base_offset(detail::calculate_base_offset<dim>(tile_index, tiling_info,
                                                       tile_size)),
        tile_index(tile_index),
        tiles_dir(detail::calculate_tiles_dir<dim>(tiling_info)),
        tiling_info(tiling_info) {}

  inline T &operator[](const size_t tile_offset) const {
    return tiled[base_offset + tile_offset];
  }

  inline T &operator[](const std::array<size_t, dim> &inner_index) const {
    size_t tile_offset = flat_index(inner_index);
    return tiled[base_offset + tile_offset];
  }

  inline T *pointer(const size_t flat_index_tile) const {
    return tiled.data() + base_offset + flat_index_tile;
  }

  template <typename... index_types>
  inline typename std::enable_if<sizeof...(index_types) == dim, T &>::type
  operator()(index_types... indices) const {
    std::array<size_t, dim> inner_index = {indices...};
    return tiled[base_offset + flat_index(inner_index)];
  }

  inline size_t size() const { return tile_size; }

  inline const std::array<size_t, dim> &get_tile_index() const {
    return tile_index;
  }

  inline const std::array<size_t, dim> &get_tiles_dir() const {
    return this->tiles_dir;
  }

  inline size_t flat_index(const std::array<size_t, dim> &coord) const {
    size_t inner_flat_index = 0;
    size_t cur_stride = 1;
    for (size_t d = 0; d < dim; d++) {
      inner_flat_index += coord[(dim - 1) - d] * cur_stride;
      cur_stride *= tiling_info[(dim - 1) - d].tile_size_dir;
    }
    return inner_flat_index;
  }
};

template <size_t dim, typename T, typename U>
tile_view<dim, T, U>
make_view_from_index(std::array<size_t, dim> index, std::vector<T, U> &tiled,
                     const tiling_configuration &tiling_info) {
  std::array<size_t, dim> tile_index;
  for (size_t d = 0; d < dim; d++) {
    tile_index[d] = index[d] / tiling_info[d].tile_size_dir;
  }
  return tile_view<dim, T, U>(tiled, tile_index, tiling_info);
}
}
