#pragma once

namespace opttmp {
namespace memory_layout {

// component type has to be indexable, and has to have a size() operator
template <typename vc_type, typename AoS_container_type, size_t num_components,
          size_t entries, size_t padding>
class struct_of_array_data {
private:
  // data in SoA form
  // const size_t num_components;
  // const size_t entries;
  // const size_t padding;
  static constexpr size_t padded_entries_per_component = entries + padding;

  typename vc_type::value_type *const data;

public:
  template <size_t component_access>
  inline typename vc_type::value_type *pointer(const size_t flat_index) const {
    constexpr size_t component_array_offset =
        component_access * padded_entries_per_component;
    // should result in single move instruction, indirect addressing: reg + reg
    // + constant
    return data + flat_index + component_array_offset;
  }

  inline typename vc_type::value_type *pointer(const size_t component_access,
                                               const size_t flat_index) const {
    // const size_t component_array_offset =
    //     component_access * padded_entries_per_component;
    // should result in single move instruction, indirect addressing: reg + reg
    // + constant
    // return data + flat_index + component_array_offset;
    return data + component_access * padded_entries_per_component + flat_index;
  }

  // careful, this returns a copy!
  template <size_t component_access>
  inline vc_type value(const size_t flat_index) const {
    return vc_type(this->pointer<component_access>(flat_index),
                   Vc::flags::element_aligned);
  }

  struct_of_array_data(const AoS_container_type &org //, size_t num_components,
                       // size_t entries, size_t padding
                       )
      : // num_components(num_components),
        // entries(entries),
        // padding(padding), padded_entries_per_component(entries + padding),
        data(new typename vc_type::value_type[num_components *
                                              padded_entries_per_component]) {
    for (size_t component = 0; component < num_components; component++) {
      for (size_t entry = 0; entry < entries; entry++) {
        data[component * padded_entries_per_component + entry] =
            org[entry * num_components + component];
        // org[entry][component];
      }
    }
    for (size_t component = 0; component < num_components; component++) {
      for (size_t entry = entries; entry < padded_entries_per_component;
           entry++) {
        data[component * padded_entries_per_component + entry] = 0.0;
      }
    }
  }

  struct_of_array_data(const size_t entries_per_component)
      : data(new typename vc_type::value_type[num_components *
                                              padded_entries_per_component]) {}

  ~struct_of_array_data() {
    if (data) {
      delete[] data;
    }
  }

  struct_of_array_data(const struct_of_array_data &other) = delete;

  struct_of_array_data(const struct_of_array_data &&other) = delete;

  struct_of_array_data &operator=(const struct_of_array_data &other) = delete;

  // write back into non-SoA style array
  void to_non_SoA(AoS_container_type &org) {
    for (size_t component = 0; component < num_components; component++) {
      for (size_t entry = 0; entry < org.size(); entry++) {
        // org[entry][component] =
        org[entry * num_components + component] =
            data[component * padded_entries_per_component + entry];
      }
    }
  }
};

} // namespace memory_layout
} // namespace opttmp
