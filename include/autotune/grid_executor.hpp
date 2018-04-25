#pragma once

#include <cinttypes>
#include <cstddef>
#include <functional>

#include "queue_thread_pool.hpp"

namespace autotune {

class grid_spec {
public:
  size_t grid_z;
  size_t grid_y;
  size_t grid_x;

  size_t block_z;
  size_t block_y;
  size_t block_x;
};

template <size_t num_threads> class grid_executor {
private:
public:
  using float_type = float;
  using double_type = double;
  using int32_type = int32_t;
  using uint32_type = uint32_t;
  using int64_type = int64_t;
  using uint64_type = uint64_t;
  static constexpr size_t types_width = 1;

  template <typename... Args>
  void operator()(std::function<void(Args...)> f, grid_spec spec) {
    autotune::queue_thread_pool<num_threads> pool;

    std::function<void(grid_spec, thread_meta)> thread_wrapper =
        [f](grid_spec spec, thread_meta meta_base) {
          for (size_t block_z = 0; block_z < spec.block_z; block_z++) {
            for (size_t block_y = 0; block_y < spec.block_y; block_y++) {
              for (size_t block_x = 0; block_x < spec.block_x;
                   block_x += types_width) {
                thread_meta meta = meta_base;
                meta.z += block_z;
                meta.y += block_y;
                meta.x += block_x;
                f(meta);
              }
            }
          }
        };

    pool.start();
    for (size_t grid_z = 0; grid_z < spec.grid_z; grid_z++) {
      for (size_t grid_y = 0; grid_y < spec.grid_y; grid_y++) {
        for (size_t grid_x = 0; grid_x < spec.grid_x; grid_x++) {
          thread_meta meta_base;
          meta_base.z = grid_z * spec.block_z;
          meta_base.y = grid_y * spec.block_y;
          meta_base.x = grid_x * spec.block_x;

          pool.enqueue_work(thread_wrapper, spec, meta_base);
        }
      }
    }
    pool.finish();
  }
};
}
