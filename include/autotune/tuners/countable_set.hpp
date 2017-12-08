#pragma once

#include "../abstract_parameter.hpp"

namespace autotune {

// class basic_parameter {
// public:
//   virtual const std::string &get_name() const = 0;
//   virtual const std::string get_value() const = 0;
// };

class countable_parameter {
public:
  virtual bool next() = 0;
  virtual bool prev() = 0;
  virtual void set_min() = 0;
  virtual size_t count_values() const = 0;
  virtual const std::string &get_name() const = 0;
  virtual const std::string get_value() const = 0;
  // virtual std::string to_parameter_source_line() = 0;
  virtual void set_initial() = 0;
  // virtual std::shared_ptr<abstract_parameter> clone() = 0;
  virtual std::shared_ptr<countable_parameter> clone_wrapper() = 0;
};

// interface-wrapper generating template
template <typename T>
class countable_parameter_wrapper : public countable_parameter {
  T p;

public:
  countable_parameter_wrapper(T p) : p(p) {}

  countable_parameter_wrapper(const countable_parameter_wrapper<T> &other)
      : p(other.p) {}

  virtual bool next() override { return p.next(); }
  virtual bool prev() override { return p.prev(); }
  virtual void set_min() override { return p.set_min(); }
  virtual size_t count_values() const override { return p.count_values(); }
  virtual const std::string &get_name() const override { return p.get_name(); }
  virtual const std::string get_value() const override { return p.get_value(); }
  // virtual std::string to_parameter_source_line() override {
  //   return p->to_parameter_source_line();
  // }
  virtual void set_initial() override { p.set_initial(); };
  // virtual std::shared_ptr<abstract_parameter> clone() override {
  //   return p->clone();
  // }

  virtual std::shared_ptr<countable_parameter> clone_wrapper() override {
    return std::make_shared<countable_parameter_wrapper<T>>(*this);
  }
};

class countable_set : public std::vector<std::shared_ptr<countable_parameter>> {

public:
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

  template <typename T> void add_parameter(T &p) {
    std::shared_ptr<countable_parameter_wrapper<T>> cloned =
        std::make_shared<countable_parameter_wrapper<T>>(p);
    this->push_back(cloned);
  }

  countable_set clone() {
    countable_set new_instance;
    for (size_t i = 0; i < this->size(); i++) {
      new_instance.push_back(this->operator[](i)->clone_wrapper());
    }
    return new_instance;
  }
};
}
