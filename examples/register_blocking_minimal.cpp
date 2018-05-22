#include "opttmp/vectorization/register_tiling.hpp"

#include <cassert>

#include <boost/align/aligned_allocator.hpp>
using boost::alignment::aligned_allocator;

#include <Vc/Vc>
using Vc::double_v;
constexpr size_t total_width = 16;
constexpr size_t K = total_width / double_v::size();

using namespace opttmp::vectorization;

constexpr size_t array_length = 8192 * 2 * 2 * 2;
constexpr size_t iterations = array_length / total_width;

int main(void) {
  std::vector<double, aligned_allocator<double, 32>> data1(array_length);
  std::vector<double, aligned_allocator<double, 32>> data2(array_length);
  for (size_t i = 0; i < array_length; i++) {
    data1[i] = static_cast<double>(i);
  }
  for (size_t i = 0; i < array_length; i++) {
    data2[i] = static_cast<double>(i);
  }
  using reg_array = register_array<double_v, K>;
  // reg_array a(data_up.data(), Vc::flags::vector_aligned);

  // reg_array b = 2.0;
  // reg_array c = 3.141;
  reg_array r = 0.0;
  // for (size_t i = 0; i < 1000000; i++) {
  //   // r += c * (a - b);
  //   r = c * (r - a);
  // }

  for (size_t i = 0; i < iterations; i++) {
    reg_array d1(data1.data() + i * total_width, Vc::flags::vector_aligned);
    reg_array d2(data2.data() + i * total_width, Vc::flags::vector_aligned);
    r += d1 * d2;
  }
  for (size_t i = 0; i < r.size(); i++) {
    std::cout << "r[" << i << "]: " << r[i] << std::endl;
  }
}
