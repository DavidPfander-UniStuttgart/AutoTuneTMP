#pragma once

#include <fstream>
#include <iostream>
#include <vector>

#include "cppjit/cppjit.hpp"
#include "cppjit/function_traits.hpp"

#include "abstract_kernel.hpp"
#include "abstract_parameter.hpp"
#include "autotune_exception.hpp"
#include "continuous_parameter.hpp"
#include "fixed_set_parameter.hpp"
#include "parameter_value_set.hpp"

namespace autotune {

// base template for the following specialization
// required to do the pack-matching in the specialization
// (required for forwardings the "Args" argument)
template <typename... Args> struct cppjit_kernel;

template <typename R, typename... Args>
class cppjit_kernel<R, cppjit::detail::pack<Args...>>
    : public abstract_kernel<R, cppjit::detail::pack<Args...>> {
private:
  cppjit::kernel<R, cppjit::detail::pack<Args...>> &internal_kernel;

public:
  cppjit_kernel(
      const std::string &kernel_name,
      cppjit::kernel<R, cppjit::detail::pack<Args...>> &internal_kernel)
      : abstract_kernel<R, cppjit::detail::pack<Args...>>(kernel_name),
        internal_kernel(internal_kernel) {}

  void set_source_inline(const std::string &source_) {
    internal_kernel.set_source_inline(source_);
  }

  void set_source_dir(const std::string &source_dir_) {
    internal_kernel.set_source_dir(source_dir_);
  }

  bool has_source() { return internal_kernel.has_source(); }

  bool has_inline_source() { return internal_kernel.has_inline_source(); }

  virtual bool is_valid_parameter_combination() override {
    auto builder = internal_kernel.get_builder();
    void *uncasted_function =
        builder->load_other_symbol("is_valid_parameter_combination");
    if (uncasted_function == nullptr) {
      return true;
    }
    bool (*decider_pointer)() =
        reinterpret_cast<decltype(decider_pointer)>(uncasted_function);
    return decider_pointer();
  }

  virtual R operator()(Args... args) override {
    if (this->parameters_changed) {
      compile();
    }
    return internal_kernel(std::forward<Args>(args)...);
  }

  // very useful overload for the kernel tuners, so that they don't have to
  // track source-related arguments
  void compile(const std::string &source_dir) {
    if (this->parameters_changed) {
      create_parameter_file();
      this->parameters_changed = false;
    }
    internal_kernel.compile(source_dir);
  }

  void compile_inline(const std::string &source) {
    if (this->parameters_changed) {
      create_parameter_file();
      this->parameters_changed = false;
    }
    internal_kernel.compile_inline(source);
  }

  virtual void compile() override {
    if (this->parameters_changed) {
      create_parameter_file();
      this->parameters_changed = false;
    }
    internal_kernel.compile();
  }

  virtual bool is_compiled() override {
    return !this->parameters_changed && internal_kernel.is_compiled();
  }

  // TODO: add parameter_set argument?
  virtual void create_parameter_file() override {
    if (!has_source() || has_inline_source()) {
      throw autotune_exception("no source available");
    }
    std::shared_ptr<cppjit::builder::builder> builder = get_builder();
    const std::string &source_dir = builder->get_source_dir();
    std::ofstream parameter_file(source_dir + "parameters.hpp");
    parameter_file << "#pragma once" << std::endl;
    for (auto &p : this->parameter_values) {
      parameter_file << "#define " << p.first << " " << p.second << "\n";
      // std::cout << "#define " << p.first << " " << p.second << std::endl;
      // parameter_file << parameters[i]->to_parameter_source_line();
    }
    parameter_file.close();
  }

  void set_builder(std::shared_ptr<cppjit::builder::builder> builder_) {
    internal_kernel.set_builder(builder_);
  }

  std::shared_ptr<cppjit::builder::builder> get_builder() {
    return internal_kernel.get_builder();
  }

  template <typename builder_class>
  std::shared_ptr<builder_class> get_builder_as() {
    return internal_kernel.template get_builder_as<builder_class>();
  }

  void clear() {
    internal_kernel.clear();
    this->verbose = false;
  }
};
}

#define AUTOTUNE_DECLARE_KERNEL(kernel_signature, kernel_name)                 \
  CPPJIT_DECLARE_KERNEL(kernel_signature, kernel_name)                         \
  namespace autotune {                                                         \
  extern cppjit_kernel<                                                        \
      cppjit::detail::function_traits<kernel_signature>::return_type,          \
      cppjit::detail::function_traits<kernel_signature>::args_type>            \
      kernel_name;                                                             \
  } /* namespace autotune */

#define AUTOTUNE_DEFINE_KERNEL(kernel_signature, kernel_name)                  \
  CPPJIT_DEFINE_KERNEL(kernel_signature, kernel_name)                          \
  namespace autotune {                                                         \
  cppjit_kernel<                                                               \
      cppjit::detail::function_traits<kernel_signature>::return_type,          \
      cppjit::detail::function_traits<kernel_signature>::args_type>            \
      kernel_name(#kernel_name, cppjit::kernel_name);                          \
  } /* namespace autotune */

#define AUTOTUNE_DECLARE_DEFINE_KERNEL(signature, kernel_name)                 \
  AUTOTUNE_DECLARE_KERNEL(signature, kernel_name)                              \
  AUTOTUNE_DEFINE_KERNEL(signature, kernel_name)
