#pragma once

#include "helpers/non_zero_seq.hpp"
#include "thread_meta.hpp"

#include <functional>
#include <iostream>

namespace autotune {
namespace detail {

class abstract_executor {
protected:
  thread_meta pool_meta;
  size_t thread_id;

public:
  virtual void operator()() = 0;
  // virtual void operator()(thread_meta &) = 0;
  void set_pool_meta(thread_meta &pool_meta) { this->pool_meta = pool_meta; }
  void set_thread_id(size_t thread_id) { this->thread_id = thread_id; }
};

template <typename... Args> class delayed_executor : public abstract_executor {
private:
  std::function<void(Args...)> f;
  std::tuple<Args...> copied_arguments;

  template <size_t... indices>
  void tuple_unwrapped(std::index_sequence<indices...>) {
    f(std::get<indices>(std::move(copied_arguments))...);
  }

public:
  delayed_executor(std::function<void(Args...)> f, Args... args)
      : f(f), copied_arguments(std::move(args)...) {}

  // ~delayed_executor() { std::cout << "destr. del. exe." << std::endl; }

  virtual void operator()() override {
    // std::cout << "del. exe. EX" << std::endl;
    tuple_unwrapped(std::make_index_sequence<sizeof...(Args)>{});
  }
};

//////////////////////////////////////////////////////////////
template <typename... Args> struct pack;

template <typename A, typename... As> struct remove_first {
  using first = A;
  using remainder = pack<As...>;
};

template <typename... Args_reduced> struct reduced_parameter_holder;

// template <typename... Args_reduced>
// struct reduced_parameter_holder<pack<Args_reduced...>> {
//   std::tuple<Args_reduced...> copied_arguments;
//   reduced_parameter_holder(Args_reduced... args)
//       : copied_arguments(std::move(args)...) {}
// };

template <typename... Args>
class delayed_executor_id : public abstract_executor
// , reduced_parameter_holder<typename remove_first<Args...>::remainder>
{
private:
  std::function<void(Args...)> f;
  std::tuple<Args...> copied_arguments;

  static constexpr size_t THREAD_ID_PLACEHOLDER = 0;

  template <size_t... indices>
  void tuple_unwrapped(autotune::index_pack<
                       indices...>) {
    f(this->thread_id, std::get<indices>(std::move(this->copied_arguments))...);
  }

public:
  delayed_executor_id(std::function<void(Args...)> f, Args... args)
    : f(f),
        copied_arguments(std::move(args)...) {
    // this->copied_arguments =
    //     std::tuple<Args_reduced...>(std::move<Args_reduced>(args), ...);
  }

  virtual void operator()() override {
    tuple_unwrapped(autotune::make_non_zero_sequence<
                    1, sizeof...(Args) - 1>{}); // std::make_integer_sequence<1,
                                                // sizeof...(Args) - 1>{}
  }
};
//////////////////////////////////////////

template <typename... Args>
class delayed_executor_meta : public abstract_executor {
private:
  std::function<void(thread_meta, Args...)> f;
  std::tuple<Args...> copied_arguments;

  template <size_t... indices>
  void tuple_unwrapped(thread_meta &pool_meta,
                       std::index_sequence<indices...>) {
    f(pool_meta, std::get<indices>(std::move(copied_arguments))...);
  }

public:
  delayed_executor_meta(std::function<void(thread_meta, Args...)> f,
                        Args... args)
      : f(f), copied_arguments(std::move(args)...) {}

  ~delayed_executor_meta() {
    std::cout << "destr. del. exe. META" << std::endl;
  }

  virtual void operator()() override {
    tuple_unwrapped(this->pool_meta,
                    std::make_index_sequence<sizeof...(Args)>{});
  }
};
} // namespace detail
} // namespace autotune
