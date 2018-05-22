#pragma once

#include "thread_meta.hpp"

#include <functional>
#include <iostream>

namespace autotune {
namespace detail {

class abstract_executor {
protected:
  thread_meta pool_meta;

public:
  virtual void operator()() = 0;
  // virtual void operator()(thread_meta &) = 0;
  void set_pool_meta(thread_meta &pool_meta) { this->pool_meta = pool_meta; }
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
