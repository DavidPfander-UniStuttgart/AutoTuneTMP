#include "geometry.hpp"
#include "kernels/calculate_stencil.hpp"
#include "kernels/m2m_interactions.hpp"
#include "taylor.hpp" //for multipole
#include "types.hpp"

#include <chrono>

using namespace octotiger;
using namespace octotiger::fmm;

space_vector one_space_vector;
multipole one_multipole;

multipole multipole_value_generator() { return one_multipole; }

space_vector space_vector_value_generator() { return one_space_vector; }

struct input_data {
  std::vector<multipole> M_ptr;
  std::vector<std::shared_ptr<std::vector<space_vector>>> com_ptr;
  std::vector<neighbor_gravity_type> all_neighbor_interaction_data;

  input_data()
      : M_ptr(PATCH_SIZE),
        all_neighbor_interaction_data(geo::direction::count()) {
    std::generate(M_ptr.begin(), M_ptr.end(), multipole_value_generator);

    std::shared_ptr<std::vector<space_vector>> com0 =
        std::shared_ptr<std::vector<space_vector>>(
            new std::vector<space_vector>(PATCH_SIZE));
    std::generate(com0->begin(), com0->end(), space_vector_value_generator);
    com_ptr.push_back(com0);
    for (geo::direction &dir : geo::direction::full_set()) {
      neighbor_gravity_type neighbor_gravity_data;
      gravity_boundary_type neighbor_boundary_data;

      // multipole data
      std::shared_ptr<std::vector<multipole>> M =
          std::shared_ptr<std::vector<multipole>>(
              new std::vector<multipole>(PATCH_SIZE));
      std::generate(M->begin(), M->end(), multipole_value_generator);
      // monopole data, not used
      std::shared_ptr<std::vector<real>> m =
          std::shared_ptr<std::vector<real>>(new std::vector<real>());

      std::shared_ptr<std::vector<space_vector>> x =
          std::shared_ptr<std::vector<space_vector>>(
              new std::vector<space_vector>(PATCH_SIZE));
      std::generate(x->begin(), x->end(), space_vector_value_generator);

      neighbor_boundary_data.M = M;
      neighbor_boundary_data.m = m;
      neighbor_boundary_data.x = x;

      // center of masses data

      neighbor_gravity_data.data = neighbor_boundary_data;
      neighbor_gravity_data.is_monopole = false;
      neighbor_gravity_data.direction = dir;
    }
  }
};

int main(void) {

  // initialize stencil globally
  octotiger::fmm::m2m_interactions::stencil = calculate_stencil();

  for (size_t i = 0; i < one_multipole.size(); i++) {
    one_multipole[i] = static_cast<double>(i) + 0.5;
  }
  for (size_t i = 0; i < one_space_vector.size(); i++) {
    one_space_vector[i] = (static_cast<double>(i) + 1.0) / 5.0;
  }

  std::vector<input_data> all_input_data(1);

  //TODO: continue verify initial values != 0
  
  for (size_t input_index = 0; input_index < all_input_data.size();
       input_index++) {
    for (size_t repetitions = 0; repetitions < 1; repetitions++) {

      std::vector<multipole> &M_ptr = all_input_data[input_index].M_ptr;
      std::vector<std::shared_ptr<std::vector<space_vector>>> &com_ptr =
          all_input_data[input_index].com_ptr;
      std::vector<neighbor_gravity_type> &all_neighbor_interaction_data =
          all_input_data[input_index].all_neighbor_interaction_data;

      gsolve_type type = gsolve_type::RHO;

      auto start_total = std::chrono::high_resolution_clock::now();

      octotiger::fmm::m2m_interactions interactor(
          M_ptr, com_ptr, all_neighbor_interaction_data, type);

      interactor.compute_interactions(); // includes boundary

      auto end_total = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> duration =
          end_total - start_total;
      std::cout << "new interaction kernel  (total w/o old non-multipole "
                   "boundary, ms): "
                << duration.count() << std::endl;

      // interactor.print_potential_expansions();
      interactor.print_center_of_masses();
    }
  }

  return 0;
}
