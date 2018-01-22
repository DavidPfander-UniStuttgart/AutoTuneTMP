#pragma once

#include "../autotune_exception.hpp"
#include "../abstract_parameter.hpp"

namespace autotune {

template <typename T> class countable_parameter_wrapper;

class countable_parameter : public abstract_parameter {
public:
  virtual bool next() = 0;
  virtual bool prev() = 0;
  virtual void set_min() = 0;
  virtual size_t count_values() const = 0;
  //virtual const std::string &get_name() const = 0;
  //virtual const std::string get_value() const = 0;
  //virtual void set_initial() = 0;
  virtual void set_random_value() = 0;
  virtual std::shared_ptr<countable_parameter> clone_wrapper() = 0;
  //template <typename T> T &get_unwrapped_parameter() {
  //  auto derived = dynamic_cast<countable_parameter_wrapper<T> *>(this);
  //  return derived->unwrapped_parameter();
  //}
};

// interface-wrapper generating template
template <typename T>
class countable_parameter_wrapper : public countable_parameter {
  T p;

public:
  // calls either constructor or copy-constructor

  countable_parameter_wrapper(T p) : p(std::move(p)) {}

  countable_parameter_wrapper(const countable_parameter_wrapper<T> &other)
      : p(other.p) {}

  virtual bool next() override { return p.next(); }
  virtual bool prev() override { return p.prev(); }
  virtual void set_min() override { return p.set_min(); }
  virtual size_t count_values() const override { return p.count_values(); }
  virtual const std::string &get_name() const override { return p.get_name(); }
  virtual const std::string get_value() const override { return p.get_value(); }
  virtual void set_initial() override { p.set_initial(); };
  virtual void set_random_value() override { return p.set_random_value(); }
  virtual std::shared_ptr<countable_parameter> clone_wrapper() override {
    return std::make_shared<countable_parameter_wrapper<T>>(*this);
  }
  T &unwrapped_parameter() { return p; }
};

class countable_set : std::vector<std::shared_ptr<countable_parameter>> {

public:
  using std::vector<std::shared_ptr<countable_parameter>>::operator[];
  using std::vector<std::shared_ptr<countable_parameter>>::size;

  countable_set() : std::vector<std::shared_ptr<countable_parameter>>() {}

  countable_set(const countable_set &other)
      : std::vector<std::shared_ptr<countable_parameter>>() {
    for (size_t i = 0; i < other.size(); i++) {
      this->push_back(other[i]->clone_wrapper());
    }
  }

  countable_set &operator=(const countable_set &other) {
    this->clear();
    for (size_t i = 0; i < other.size(); i++) {
      this->push_back(other[i]->clone_wrapper());
    }
    return *this;
  }

  template <typename T> T &get_by_name(const std::string &name) {
    for (auto p : *this) {
      if (p->get_name().compare(name) == 0) {
        return p->get_unwrapped_parameter<T>();
      }
    }
    throw autotune_exception("parameter not in set");
  }

  template <typename T> void add_parameter(T &p) {
    std::shared_ptr<countable_parameter_wrapper<T>> cloned =
        std::make_shared<countable_parameter_wrapper<T>>(p);
    this->push_back(cloned);
  }

  template <typename T, typename... Ts> void emplace_parameter(Ts &&... args) {
    T p(std::forward<Ts>(args)...);
    std::shared_ptr<countable_parameter_wrapper<T>> wrapper =
        std::make_shared<countable_parameter_wrapper<T>>(std::move(p));
    this->push_back(wrapper);
  }

  void print_values() {
    std::cout << "parameter name  | ";
    bool first = true;
    for (auto &p : *this) {
      if (!first) {
        std::cout << ", ";
      } else {
        first = false;
      }
      std::cout << p->get_name();
      int64_t padding = std::max(p->get_name().size(), p->get_value().size()) -
                        p->get_name().size();
      if (padding > 0) {
        std::stringstream ss;
        for (int64_t i = 0; i < padding; i++) {
          ss << " ";
        }
        std::cout << ss.str();
      }
    }
    std::cout << std::endl;
    std::cout << "parameter value | ";
    first = true;
    for (auto &p : *this) {
      if (!first) {
        std::cout << ", ";
      } else {
        first = false;
      }
      std::cout << p->get_value();
      int64_t padding = std::max(p->get_name().size(), p->get_value().size()) -
                        p->get_value().size();
      if (padding > 0) {
        std::stringstream ss;
        for (int64_t i = 0; i < padding; i++) {
          ss << " ";
        }
        std::cout << ss.str();
      }
    }
    std::cout << std::endl;
  }
};
}
