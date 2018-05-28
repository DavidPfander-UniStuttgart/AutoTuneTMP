#pragma once

#include <fstream>
#include <iostream>
#include <vector>

#include "cppjit/cppjit.hpp"
#include "cppjit/function_traits.hpp"

#include "abstract_kernel.hpp"
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
  cppjit::kernel<R, cppjit::detail::pack<Args...>> internal_kernel;
  void (*set_meta_pointer)(thread_meta);

public:
  cppjit_kernel(const std::string &kernel_name)
      : abstract_kernel<R, cppjit::detail::pack<Args...>>(kernel_name),
        internal_kernel(kernel_name), set_meta_pointer(nullptr) {}

  cppjit_kernel(const std::string &kernel_name,
                const std::string &kernel_src_dir)
      : abstract_kernel<R, cppjit::detail::pack<Args...>>(kernel_name),
        internal_kernel(kernel_name, kernel_src_dir),
        set_meta_pointer(nullptr) {}

  cppjit_kernel(cppjit_kernel<R, cppjit::detail::pack<Args...>> &other)
      : abstract_kernel<R, cppjit::detail::pack<Args...>>(other),
        internal_kernel(other.internal_kernel), set_meta_pointer(nullptr) {}

  void set_source_inline(const std::string &source_) {
    internal_kernel.set_source_inline(source_);
  }

  void set_source_dir(const std::string &source_dir_) {
    internal_kernel.set_source_dir(source_dir_);
  }

  bool has_source() { return internal_kernel.has_source(); }

  bool has_inline_source() { return internal_kernel.has_inline_source(); }

  void *load_other_symbol(const std::string &symbol_name) {
    auto builder = internal_kernel.get_builder();
    void *uncasted_function = builder->load_other_symbol(symbol_name);
    if (uncasted_function == nullptr) {
      throw autotune_exception(std::string("unable to load symbol: \"") +
                               symbol_name + std::string("\""));
    }
    return uncasted_function;
  }

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
    if (!is_compiled()) {
      compile();
    }
    return internal_kernel(std::forward<Args>(args)...);
  }

  // very useful overload for the kernel tuners, so that they don't have to
  // track source-related arguments
  void compile(const std::string &source_dir) {
    create_parameter_file();
    this->parameters_changed = false;
    internal_kernel.compile(source_dir);
  }

  void compile_inline(const std::string &source) {
    create_parameter_file();
    this->parameters_changed = false;
    internal_kernel.compile_inline(source);
  }

  virtual void compile() override {
    create_parameter_file();
    this->parameters_changed = false;
    internal_kernel.compile();
  }

  virtual bool is_compiled() override {
    // std::cout << "parameters_changed: " << this->parameters_changed
    //           << std::endl;
    // std::cout << "internal_kernel.is_compiled(): "
    // << internal_kernel.is_compiled() << std::endl;
    return !this->parameters_changed && internal_kernel.is_compiled();
  }

  // TODO: add parameter_set argument?
  virtual void create_parameter_file() override {
    if (!has_source() || has_inline_source()) {
      throw autotune_exception("no source available");
    }

    std::shared_ptr<cppjit::builder::builder> builder = get_builder();
    // needed, otherwise cannot create parameter file at the right place
    builder->make_compile_dir();
    const std::string &compile_dir = builder->get_compile_dir();
    std::ofstream parameter_file(compile_dir + "parameters.hpp");
    parameter_file << "#pragma once" << std::endl;
    parameter_file << "#include \"autotune_kernel.hpp\"" << std::endl;

    for (auto &p : this->parameter_values) {
      parameter_file << "#define " << p.first << " " << p.second << "\n";
    }
    parameter_file.close();
    std::ofstream autotune_kernel_file(compile_dir + "autotune_kernel.hpp");
    autotune_kernel_file << "#pragma once" << std::endl;
    autotune_kernel_file << "#include \"cppjit_kernel.hpp\"" << std::endl;
    autotune_kernel_file << "#include \"parameters.hpp\"" << std::endl;
    autotune_kernel_file << "#define AUTOTUNE_EXPORT CPPJIT_EXPORT"
                         << std::endl;
    autotune_kernel_file.close();
  }

  void set_builder(std::shared_ptr<cppjit::builder::builder> builder_) {
    internal_kernel.set_builder(builder_);
  }

  std::shared_ptr<cppjit::builder::builder> get_builder() {
    return internal_kernel.get_builder();
  }

  template <typename builder_class> builder_class &get_builder() {
    return internal_kernel.template get_builder<builder_class>();
  }

  void clear() {
    abstract_kernel<R, cppjit::detail::pack<Args...>>::clear();
    internal_kernel.clear();
    this->verbose = false;
  }

  virtual void set_verbose(bool verbose) override {
    this->verbose = verbose;
    internal_kernel.set_verbose(verbose);
  }

  virtual abstract_kernel<R, cppjit::detail::pack<Args...>> *clone() override {
    return new cppjit_kernel<R, cppjit::detail::pack<Args...>>(*this);
  }

  // important: will forget data in case of a recompile
  // due to data being stored within shared object which is unloaded and
  // replaced
  virtual void set_meta(thread_meta meta) override {
    if (!is_compiled()) {
      compile();
    }
    if (!set_meta_pointer) {
      void *uncasted_function = load_other_symbol("set_meta");
      set_meta_pointer =
          reinterpret_cast<decltype(set_meta_pointer)>(uncasted_function);
    }
    // std::cout << "set_meta pointer: "
    //           << reinterpret_cast<void *>(set_meta_pointer) << std::endl;
    set_meta_pointer(meta);
  };

  virtual thread_meta get_meta() {
    throw autotune_exception("not available for cppjit kernel");
  }
};
} // namespace autotune

#define AUTOTUNE_DECLARE_KERNEL(kernel_signature, kernel_name)                 \
  namespace autotune {                                                         \
  extern cppjit_kernel<                                                        \
      cppjit::detail::function_traits<kernel_signature>::return_type,          \
      cppjit::detail::function_traits<kernel_signature>::args_type>            \
      kernel_name;                                                             \
  } /* namespace autotune */

#define AUTOTUNE_DEFINE_KERNEL_NO_SRC(kernel_signature, kernel_name)           \
  namespace autotune {                                                         \
  cppjit_kernel<                                                               \
      cppjit::detail::function_traits<kernel_signature>::return_type,          \
      cppjit::detail::function_traits<kernel_signature>::args_type>            \
      kernel_name(#kernel_name);                                               \
  } /* namespace autotune */

#define AUTOTUNE_DEFINE_KERNEL(kernel_signature, kernel_name, kernel_src_dir)  \
  namespace autotune {                                                         \
  cppjit_kernel<                                                               \
      cppjit::detail::function_traits<kernel_signature>::return_type,          \
      cppjit::detail::function_traits<kernel_signature>::args_type>            \
      kernel_name(#kernel_name, kernel_src_dir);                               \
  } /* namespace autotune */

#define AUTOTUNE_KERNEL_NO_SRC(signature, kernel_name)                         \
  AUTOTUNE_DECLARE_KERNEL(signature, kernel_name)                              \
  AUTOTUNE_DEFINE_KERNEL_NO_SRC(signature, kernel_name)

#define AUTOTUNE_KERNEL(signature, kernel_name, kernel_src_dir)                \
  AUTOTUNE_DECLARE_KERNEL(signature, kernel_name)                              \
  AUTOTUNE_DEFINE_KERNEL(signature, kernel_name, kernel_src_dir)
