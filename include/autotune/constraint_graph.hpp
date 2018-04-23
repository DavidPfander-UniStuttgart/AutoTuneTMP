#pragma once

#include <map>
#include <set>
#include <vector>

#include "abstract_parameter.hpp"
#include "autotune_exception.hpp"
#include "parameter_superset.hpp"
#include "tuners/countable_set.hpp"

namespace autotune {

// namespace detail {

// // // e.g. wrapper for countable_sets -> inner type will be
// countable_parameter
// // class parameter_wrapper {
// // private:
// //   abstract_parameter &parameter;

// // public:
// //   template <typename set_parameter_type>
// //   thin_wrapper(set_parameter_type set_parameter)
// //       : parameter(set_parameter->get_underlying_abstract_parameter()) {}
// //   abstract_parameter &get() { return parameter; }
// // };

// class parameter_superset : std::map<std::string, void *> {
// private:
//   // std::map<std::string, std::shared_ptr<abstract_parameter>> map;

//   template <typename T> void add_parameters(T &parameter_set) {
//     for (size_t i = 0; i < parameter_set.size(); i++) {
//       if (this->count(parameter_set[i]->get_name()) != 0) {
//         throw autotune_exception(std::string("parameter in multiple sets: ")
//         +
//                                  parameter_set[i]->get_name());
//       }
//       this->operator[](parameter_set[i]->get_name()) =
//           parameter_set[i]->get_void_ptr();
//     }
//   }

// public:
//   template <typename... Ts> parameter_superset(Ts &... parameter_sets) {
//     (this->add_parameters(parameter_sets), ...);
//   }

//   using std::map<std::string, void *>::size;

//   template <typename T> T &get_as(const std::string &name) {
//     T *casted = static_cast<T *>(this->operator[](name));
//     return *casted;
//   }
// };
// }

class constraint_graph {
private:
  std::map<std::string, std::set<std::string>> conditions;
  std::map<std::string, std::function<void(parameter_superset &)>> functors;
  std::vector<std::string> ordered_deps;
  parameter_superset super;

  // void check_disjunct(const std::initializer_list<std::string> &left,
  //                     const std::initializer_list<std::string> &right) {
  //   for (const std::string &left_element : left) {
  //     for (const std::string &right_element : right) {
  //       if (left_element.compare(right_element) == 0) {
  //         throw autotune_exception("parameter dependency of itself");
  //       }
  //     }
  //   }
  // }

  // // "a after b" specified, ensure that "b after a" not given as well
  // void check_contradicts(const std::string &then_parameter,
  //                        const std::string &before_parameter) {
  //   for (auto &c : conditions) {
  //     if (c.first.compare(before_parameter) == 0) {
  //       for (size_t i = 0; i < c.second.size(); i++) {
  //         if (c.second[i].compare(then_parameter) == 0) {
  //           throw autotune_exception(
  //               "parameter dependency cycle detected"); // error:
  //               contradiction
  //         }
  //       }
  //     }
  //   }
  // }

  void create_graph() {
    ordered_deps.clear();

    // count parameters that have  mentioned in rules
    std::vector<std::string> parameters_to_place;
    for (auto &c : conditions) {
      parameters_to_place.push_back(c.first);
    }
    std::set<std::string> satisfied;
    // add all parameters that have no conditions, but are mentioned as
    // conditions
    for (auto &c : conditions) {
      for (const std::string &parameter : c.second) {
        auto it_is_condition = std::find(parameters_to_place.begin(),
                                         parameters_to_place.end(), parameter);
        if (it_is_condition != parameters_to_place.end()) {
          continue;
        }
        // auto it_already_satisfied = std::find(satisfied.begin(), satisfied);
        satisfied.insert(parameter);
      }
    }

    while (parameters_to_place.size() > 0) {
      std::vector<std::string> parameters_placed;
      for (const std::string &to_place : parameters_to_place) {
        // check whether all conditions are satisfied
        bool conditions_satisfied = true;
        for (const std::string &c : conditions[to_place]) {
          auto it = std::find(satisfied.begin(), satisfied.end(), c);
          if (it == satisfied.end()) {
            // not satisfied
            conditions_satisfied = false;
            break;
          }
        }
        if (conditions_satisfied) {
          ordered_deps.push_back(to_place);
          satisfied.insert(to_place);
          parameters_placed.push_back(to_place);
        }
      }
      // remove all placed parameters from the list of parameters still to do
      for (const std::string &to_erase : parameters_placed) {
        parameters_to_place.erase(std::find(
            parameters_to_place.begin(), parameters_to_place.end(), to_erase));
      }
      if (parameters_placed.size() == 0) {
        throw autotune_exception("complex parameter dependency detected");
      }
    }
  }

public:
  // template <typename... Ts> void apply_dependencies(Ts... parameter_sets) {
  //   parameter_superset super(parameter_sets...);

  //   // create dependency graph from conditions
  //   create_graph();

  //   for (size_t i = 0; i < ordered_deps.size(); i++) {
  //     std::cout << "applying dep: " << ordered_deps[i] << std::endl;
  //     functors[ordered_deps[i]](super);
  //   }
  // }

  template <typename parameter_interface>
  void add_parameters(parameter_interface &set) {
    super.add_parameters(set);
  }

  void adjust() {
    // create dependency graph from conditions
    create_graph();

    for (size_t i = 0; i < ordered_deps.size(); i++) {
      std::cout << "applying dep: " << ordered_deps[i] << std::endl;
      functors[ordered_deps[i]](super);
    }
  }

  void clear() { super = parameter_superset(); }

  // parameters satisfied afterwards, parameters depending, functional to fix
  // parameters
  template <typename F>
  void add_constraint(std::string affected_parameter,
                      std::initializer_list<std::string> dep_parameters, F f) {
    // // ensure that parameter sets are disjunct
    // check_disjunct({affected_parameter}, dep_parameters);

    // // check for contradiction ("a before b", but also "b before a"
    // specified)
    // // for (size_t i = 0; i < affected_parameters.size(); i++) {
    // //   for (size_t j = 0; j < dep_parameters.size(); j++) {
    // // for (const std::string &affected_parameter : affected_parameters) {
    // for (const std::string &dep_parameter : dep_parameters) {
    //   check_contradicts(affected_parameter, dep_parameter);
    // }
    // // }

    if (conditions.count(affected_parameter) != 0) {
      throw autotune_exception("constraint for parameter already specified");
    }

    // ensure dependency is not already set up
    if (functors.count(affected_parameter) != 0) {
      throw autotune_exception("multiple functors specified for parameter");
    }

    std::set<std::string> dep_parameter_set;
    for (const std::string &d : dep_parameters) {
      dep_parameter_set.insert(d);
    }

    // add new conditions
    functors[affected_parameter] = f;
    conditions[affected_parameter] = dep_parameter_set;

    create_graph();
    // for (const std::string &dep_parameter : dep_parameters) {
    //   if (conditions.find(affected_parameter) == conditions.end()) {
    //     conditions[affected_parameter] = std::vector<std::string>();
    //   }
    //   auto &parameter_deps = conditions[affected_parameter];
    //   auto it = std::find(parameter_deps.begin(), parameter_deps.end(),
    //                       dep_parameter);
    //   if (it != parameter_deps.end()) {
    //     continue;
    //   }

    //   conditions[affected_parameter].push_back(dep_parameter);
    // }
  }
};
}
