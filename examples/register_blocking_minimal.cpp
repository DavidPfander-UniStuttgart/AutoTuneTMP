#include "opttmp/vectorization/register_tiling.hpp"

#include <cassert>

#include <boost/align/aligned_allocator.hpp>
using boost::alignment::aligned_allocator;

#include <Vc/Vc>
using Vc::double_v;
constexpr size_t total_width = 16;
constexpr size_t blocking = total_width / double_v::size();

using namespace opttmp::vectorization;

constexpr size_t array_length = 8192 * 2 * 2 * 2;
constexpr size_t iterations = array_length / total_width;

int main(void) {
  std::vector<double, aligned_allocator<double, 32>> A(array_length);
  std::vector<double, aligned_allocator<double, 32>> B(array_length);
  for (size_t i = 0; i < array_length; i++) {
    A[i] = static_cast<double>(i);
  }
  for (size_t i = 0; i < array_length; i++) {
    B[i] = static_cast<double>(i);
  }

  using reg_array = register_array<double_v, blocking>;
  reg_array r = 0.0;
  for (size_t i = 0; i < iterations; i++) {
    reg_array a_reg(A.data() + i * total_width, Vc::flags::vector_aligned);
    reg_array b_reg(B.data() + i * total_width, Vc::flags::vector_aligned);
    r += a_reg * b_reg;
  }
  for (size_t i = 0; i < r.size(); i++) {
    std::cout << "r[" << i << "]: " << r[i] << std::endl;
  }
}

  // reg_array a(data_up.data(), Vc::flags::vector_aligned);
  // reg_array b = 2.0;
  // reg_array c = 3.141;
	  // for (size_t i = 0; i < 1000000; i++) {
  //   // r += c * (a - b);
  //   r = c * (r - a);
  // }
