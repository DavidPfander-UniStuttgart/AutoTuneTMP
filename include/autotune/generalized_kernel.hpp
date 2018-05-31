#pragma once

#include <fstream>
#include <iostream>
#include <vector>

#include "cppjit/function_traits.hpp"

#include "abstract_kernel.hpp"
#include "autotune_exception.hpp"
#include "continuous_parameter.hpp"
#include "fixed_set_parameter.hpp"
#include "parameter_value_set.hpp"
#include "thread_meta.hpp"

namespace autotune {

// base template for the following specialization
// required to do the pack-matching in the specialization
// (required for forwardings the "Args" argument)
template <typename... Args> struct generalized_kernel;

template <typename R, typename... Args>
class generalized_kernel<R, cppjit::detail::pack<Args...>>
    : public abstract_kernel<R, cppjit::detail::pack<Args...>> {
private:
  std::function<R(Args...)> internal_kernel;
  std::function<bool(parameter_value_set &)>
      valid_parameter_combination_functor;
  std::function<void(parameter_value_set &)>
      create_parameter_combination_functor;
  thread_meta meta;

public:
  generalized_kernel(
      const std::string &kernel_name
      //                     ,std::function<R(Args...)> &internal_kernel
      )
      : abstract_kernel<R, cppjit::detail::pack<Args...>>(kernel_name)
  //        ,internal_kernel(internal_kernel)
  {
    valid_parameter_combination_functor = [](parameter_value_set &) {
      return true;
    };
  }

  void set_valid_parameter_combination_functor(
      std::function<bool(parameter_value_set &)>
          valid_parameter_combination_functor) {
    this->valid_parameter_combination_functor =
        valid_parameter_combination_functor;
  }

  virtual bool is_valid_parameter_combination() override {
    if (valid_parameter_combination_functor) {
      return valid_parameter_combination_functor(this->parameter_values);
    }
    // if not tests is specified, every combination is accepted
    return true;
  }

  void set_kernel_functor(std::function<R(Args...)> internal_kernel) {
    this->internal_kernel = internal_kernel;
  }

  virtual R operator()(Args... args) override {
    return internal_kernel(std::forward<Args>(args)...);
  }

  virtual void compile() override {}

  virtual bool is_compiled() override { return true; }

  void
  set_create_parameter_file_functor(std::function<void(parameter_value_set &)>
                                        create_parameter_combination_functor) {
    this->create_parameter_combination_functor =
        create_parameter_combination_functor;
  }

  virtual void create_parameter_file() override {
    create_parameter_combination_functor(this->parameter_values);
  }

  virtual abstract_kernel<R, cppjit::detail::pack<Args...>> *clone() override {
    return new generalized_kernel<R, cppjit::detail::pack<Args...>>(*this);
  }

  virtual void set_meta(thread_meta meta) {
    std::cout << "kernel meta set" << std::endl;
    std::cout << "set meta.z: " << meta.z << std::endl;
    std::cout << "set meta.y: " << meta.y << std::endl;
    std::cout << "set meta.x: " << meta.x << std::endl;
    this->meta = meta;
  }

  virtual thread_meta get_meta() override { return meta; }
};
} // namespace autotune

#define AUTOTUNE_DECLARE_GENERALIZED_KERNEL(kernel_signature, kernel_name)     \
  namespace autotune {                                                         \
  extern generalized_kernel<                                                   \
      cppjit::detail::function_traits<kernel_signature>::return_type,          \
      cppjit::detail::function_traits<kernel_signature>::args_type>            \
      kernel_name;                                                             \
  } /* namespace autotune */

#define AUTOTUNE_DEFINE_GENERALIZED_KERNEL(kernel_signature, kernel_name)      \
  namespace autotune {                                                         \
  generalized_kernel<                                                          \
      cppjit::detail::function_traits<kernel_signature>::return_type,          \
      cppjit::detail::function_traits<kernel_signature>::args_type>            \
      kernel_name(#kernel_name);                                               \
  } /* namespace autotune */

#define AUTOTUNE_GENERALIZED_KERNEL(signature, kernel_name)                    \
  AUTOTUNE_DECLARE_GENERALIZED_KERNEL(signature, kernel_name)                  \
  AUTOTUNE_DEFINE_GENERALIZED_KERNEL(signature, kernel_name)
