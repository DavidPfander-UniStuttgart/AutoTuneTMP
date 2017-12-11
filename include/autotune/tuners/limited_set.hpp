#pragma once

#include "../abstract_parameter.hpp"

namespace autotune {

class limited_parameter {
public:
  virtual void set_min() = 0;
  virtual void set_max() = 0;
  virtual double get_min() const = 0;
  virtual double get_max() const = 0;
  virtual const std::string &get_name() const = 0;
  virtual const std::string get_value() const = 0;
  virtual bool set_value(double new_value) = 0;
  virtual void set_initial() = 0;
  virtual bool is_integer_parameter() const = 0;
  virtual std::shared_ptr<limited_parameter> clone_wrapper() = 0;
};

// interface-wrapper generating template
template <typename T>
class limited_parameter_wrapper : public limited_parameter {
  T p;

public:
  limited_parameter_wrapper(T p) : p(p) {}

  limited_parameter_wrapper(const limited_parameter_wrapper<T> &other)
      : p(other.p) {}

  virtual void set_min() override { return p.set_min(); }
  virtual void set_max() override { return p.set_max(); }
  virtual double get_min() const override { return p.get_min(); };
  virtual double get_max() const override { return p.get_max(); };
  virtual const std::string &get_name() const override { return p.get_name(); }
  virtual const std::string get_value() const override { return p.get_value(); }
  virtual bool set_value(double new_value) override {
    std::cout << "new value: " << new_value << std::endl;
    return p.set_value(new_value);
  }
  virtual void set_initial() override { p.set_initial(); };
  virtual bool is_integer_parameter() const { return p.is_integer_parameter(); }
  virtual std::shared_ptr<limited_parameter> clone_wrapper() override {
    return std::make_shared<limited_parameter_wrapper<T>>(*this);
  }
};

class limited_set : public std::vector<std::shared_ptr<limited_parameter>> {

public:
  limited_set() : std::vector<std::shared_ptr<limited_parameter>>() {}

  limited_set(const limited_set &other)
      : std::vector<std::shared_ptr<limited_parameter>>() {
    for (size_t i = 0; i < other.size(); i++) {
      this->push_back(other[i]->clone_wrapper());
    }
  }

  limited_set &operator=(const limited_set &other) {
    this->clear();
    for (size_t i = 0; i < other.size(); i++) {
      this->push_back(other[i]->clone_wrapper());
    }
    return *this;
  }

  template <typename T> void add_parameter(T &p) {
    std::shared_ptr<limited_parameter_wrapper<T>> cloned =
        std::make_shared<limited_parameter_wrapper<T>>(p);
    this->push_back(cloned);
  }

  limited_set clone() {
    limited_set new_instance;
    for (size_t i = 0; i < this->size(); i++) {
      new_instance.push_back(this->operator[](i)->clone_wrapper());
    }
    return new_instance;
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
    }
    std::cout << std::endl;
  }
};
}
