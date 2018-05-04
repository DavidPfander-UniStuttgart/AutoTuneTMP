#include <algorithm>
#include <functional>
#include <vector>

#include <Vc/Vc>
using Vc::double_v;

using namespace std;

constexpr size_t dims = 5;

AUTOTUNE_EXPORT void square_vector(vector<double> &origin, vector<double> &dest) {
  // transform(origin.begin(), origin.end(), origin.begin(), dest.begin(),
  //           multiplies<double>());
  const size_t it_size = origin.size();
  double *org_base = origin.data();
  double *dest_base = dest.data();
  for (size_t i = 0; i < it_size; i += 1 * Vc::double_v::size()) {

    // dest[i + 0] = origin[i + 0] * origin[i + 0] * origin[i + 0] *
    //               origin[i + 0] * origin[i + 0] * origin[i + 0];
    // dest[i + 1] = origin[i + 1] * origin[i + 1] * origin[i + 1] *
    //               origin[i + 1] * origin[i + 1] * origin[i + 1];
    // dest[i + 2] = origin[i + 2] * origin[i + 2] * origin[i + 2] *
    //               origin[i + 2] * origin[i + 2] * origin[i + 2];
    // dest[i + 3] = origin[i + 3] * origin[i + 3] * origin[i + 3] *
    //               origin[i + 3] * origin[i + 3] * origin[i + 3];
    // dest[i + 4] = origin[i + 4] * origin[i + 4] * origin[i + 4] *
    //               origin[i + 4] * origin[i + 4] * origin[i + 4];
    // dest[i + 5] = origin[i + 5] * origin[i + 5] * origin[i + 5] *
    //               origin[i + 5] * origin[i + 5] * origin[i + 5];
    // dest[i + 6] = origin[i + 6] * origin[i + 6] * origin[i + 6] *
    //               origin[i + 6] * origin[i + 6] * origin[i + 6];
    // dest[i + 7] = origin[i + 7] * origin[i + 7] * origin[i + 7] *
    //               origin[i + 7] * origin[i + 7] * origin[i + 7];

    {
      double_v factor(org_base + (i + 0 * Vc::double_v::size()),
                      Vc::flags::element_aligned);
      double_v result = factor;
      result *= factor;
      result *= factor;
      result *= factor;
      result *= factor;
      result *= factor;
      result *= factor;
      result *= factor;
      result *= factor;
      result *= factor;
      result *= factor;
      result *= factor;
      result *= factor;
      result *= factor;
      result *= factor;
      result *= factor;
      result.memstore(dest_base + (i + 0 * Vc::double_v::size()),
                      Vc::flags::element_aligned);
    }
    //     {
    //   double_v factor(org_base + (i + 1 * Vc::double_v::size()),
    //                   Vc::flags::element_aligned);
    //   double_v result = factor;
    //   result *= factor;
    //   result *= factor;
    //   result *= factor;
    //   result *= factor;
    //   result *= factor;
    //   result *= factor;
    //   result *= factor;
    //   result *= factor;
    //   result *= factor;
    //   result *= factor;
    //   result *= factor;
    //   result *= factor;
    //   result *= factor;
    //   result *= factor;
    //   result *= factor;
    //   result.memstore(dest_base + (i + 1 * Vc::double_v::size()),
    //                   Vc::flags::element_aligned);
    // }

    // {
    //   double_v factor(&origin[i + 1 * Vc::double_v::size()],
    //                   Vc::flags::element_aligned);
    //   double_v result = factor * factor * factor * factor * factor * factor;
    //   result.memstore(&dest[i + 1 * Vc::double_v::size()],
    //                   Vc::flags::element_aligned);
    // }
    // {
    //   double_v factor(&origin[i + 2 * Vc::double_v::size()],
    //                   Vc::flags::element_aligned);
    //   double_v result = factor * factor * factor * factor * factor * factor;
    //   result.memstore(&dest[i + 2 * Vc::double_v::size()],
    //                   Vc::flags::element_aligned);
    // }
    // {
    //   double_v factor(&origin[i + 3 * Vc::double_v::size()],
    //                   Vc::flags::element_aligned);
    //   double_v result = factor * factor * factor * factor * factor * factor;
    //   result.memstore(&dest[i + 3 * Vc::double_v::size()],
    //                   Vc::flags::element_aligned);
    // }
    // {
    //   double_v factor(&origin[i + 4 * Vc::double_v::size()],
    //                   Vc::flags::element_aligned);
    //   double_v result = factor * factor * factor * factor * factor * factor;
    //   result.memstore(&dest[i + 4 * Vc::double_v::size()],
    //                   Vc::flags::element_aligned);
    // }
    // {
    //   double_v factor(&origin[i + 5 * Vc::double_v::size()],
    //                   Vc::flags::element_aligned);
    //   double_v result = factor * factor * factor * factor * factor * factor;
    //   result.memstore(&dest[i + 5 * Vc::double_v::size()],
    //                   Vc::flags::element_aligned);
    // }
    // {
    //   double_v factor(&origin[i + 6 * Vc::double_v::size()],
    //                   Vc::flags::element_aligned);
    //   double_v result = factor * factor * factor * factor * factor * factor;
    //   result.memstore(&dest[i + 6 * Vc::double_v::size()],
    //                   Vc::flags::element_aligned);
    // }
    // {
    //   double_v factor(&origin[i + 7 * Vc::double_v::size()],
    //                   Vc::flags::element_aligned);
    //   double_v result = factor * factor * factor * factor * factor * factor;
    //   result.memstore(&dest[i + 7 * Vc::double_v::size()],
    //                   Vc::flags::element_aligned);
    // }

    // double_v factor_0(&origin[i + 0 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // double_v factor_1(&origin[i + 1 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // double_v factor_2(&origin[i + 2 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // double_v factor_3(&origin[i + 3 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // double_v factor_4(&origin[i + 4 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // double_v factor_5(&origin[i + 5 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // double_v factor_6(&origin[i + 6 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // double_v factor_7(&origin[i + 7 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // double_v factor_8(&origin[i + 8 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // double_v factor_9(&origin[i + 9 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // double_v factor_10(&origin[i + 10 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // double_v factor_11(&origin[i + 11 * Vc::double_v::size()],
    // Vc::flags::element_aligned);

    // double_v result_0 = factor_0;
    // double_v result_1 = factor_1;
    // // double_v result_2 = factor_2;
    // // double_v result_3 = factor_3;
    // // double_v result_4 = factor_4;
    // // double_v result_5 = factor_5;
    // // double_v result_6 = factor_6;
    // // double_v result_7 = factor_7;
    // // double_v result_8 = factor_8;
    // // double_v result_9 = factor_9;
    // // double_v result_10 = factor_10;
    // // double_v result_11 = factor_11;

    // result_0 *= factor_0;
    // result_1 *= factor_1;
    // // result_2 *= factor_2;
    // // result_3 *= factor_3;
    // // result_4 *= factor_4;
    // // result_5 *= factor_5;
    // // result_6 *= factor_6;
    // // result_7 *= factor_7;
    // // result_8 *= factor_8;
    // // result_9 *= factor_9;
    // // result_10 *= factor_10;
    // // result_11 *= factor_11;

    // result_0 *= factor_0;
    // result_1 *= factor_1;
    // // result_2 *= factor_2;
    // // result_3 *= factor_3;
    // // result_4 *= factor_4;
    // // result_5 *= factor_5;
    // // result_6 *= factor_6;
    // // result_7 *= factor_7;
    // // result_8 *= factor_8;
    // // result_9 *= factor_9;
    // // result_10 *= factor_10;
    // // result_11 *= factor_11;

    // result_0 *= factor_0;
    // result_1 *= factor_1;
    // // result_2 *= factor_2;
    // // result_3 *= factor_3;
    // // result_4 *= factor_4;
    // // result_5 *= factor_5;
    // // result_6 *= factor_6;
    // // result_7 *= factor_7;
    // // result_8 *= factor_8;
    // // result_9 *= factor_9;
    // // result_10 *= factor_10;
    // // result_11 *= factor_11;

    // result_0 *= factor_0;
    // result_1 *= factor_1;
    // // result_2 *= factor_2;
    // // result_3 *= factor_3;
    // // result_4 *= factor_4;
    // // result_5 *= factor_5;
    // // result_6 *= factor_6;
    // // result_7 *= factor_7;
    // // result_8 *= factor_8;
    // // result_9 *= factor_9;
    // // result_10 *= factor_10;
    // // result_11 *= factor_11;

    // result_0 *= factor_0;
    // result_1 *= factor_1;
    // // result_2 *= factor_2;
    // // result_3 *= factor_3;
    // // result_4 *= factor_4;
    // // result_5 *= factor_5;
    // // result_6 *= factor_6;
    // // result_7 *= factor_7;
    // // result_8 *= factor_8;
    // // result_9 *= factor_9;
    // // result_10 *= factor_10;
    // // result_11 *= factor_11;

    // result_0.memstore(&dest[i + 0 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // result_1.memstore(&dest[i + 1 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // result_2.memstore(&dest[i + 2 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // result_3.memstore(&dest[i + 3 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // result_4.memstore(&dest[i + 4 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // result_5.memstore(&dest[i + 5 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // result_6.memstore(&dest[i + 6 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // result_7.memstore(&dest[i + 7 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // result_8.memstore(&dest[i + 8 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // result_9.memstore(&dest[i + 9 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // result_10.memstore(&dest[i + 10 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
    // // result_11.memstore(&dest[i + 11 * Vc::double_v::size()],
    // Vc::flags::element_aligned);
  }
}
