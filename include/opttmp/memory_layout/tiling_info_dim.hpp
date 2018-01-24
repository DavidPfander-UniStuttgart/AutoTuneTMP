#pragma once

namespace memory_layout {
struct tiling_info_dim {
  size_t tile_size_dir;
  size_t stride;
};

class tiling_configuration {
private:
  std::vector<tiling_info_dim> infos;

public:
  tiling_configuration(std::initializer_list<tiling_info_dim> infos) {
    this->infos.insert(this->infos.end(), infos.begin(), infos.end());
  }
  tiling_info_dim operator[](size_t i) const { return infos[i]; }
  size_t size() const { return infos.size(); }
};
}
