#pragma once

#include "../abstract_parameter.hpp"

namespace autotune {

namespace tree {

template <typename parameter_interface> class node {
public:
  std::unordered_map<std::string, node<parameter_interface>> children;

  void insert(parameter_interface &parameters, size_t cur_index) {
    if (cur_index == parameters.size()) {
      return;
    }

    if (children.find(parameters[cur_index]->get_value()) == children.end()) {
      children[parameters[cur_index]->get_value()] =
          node<parameter_interface>();
    } else {
    }
    r.insert(parameters, cur_index + 1);
  }

  bool contains(parameter_interface &candidate) {
    return r.contains(candidate, 0);
  }
};

template <typename parameter_interface>
class leaf : public node<parameter_interface> {};
}

template <typename parameter_interface> class root_node {
private:
  node<parameter_interface> r;

public:
  void insert(parameter_interface &parameters) { r.insert(parameters); }

  bool contains(parameter_interface &candidate) {
    return r.contains(candidate, 0);
  }
};

class countable_parameter {
public:
  virtual bool next() = 0;
  virtual bool prev() = 0;
  virtual void set_min() = 0;
  virtual size_t count_values() const = 0;
  virtual const std::string &get_name() const = 0;
  virtual const std::string get_value() const = 0;
  virtual void set_initial() = 0;
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
  virtual void set_initial() override { p.set_initial(); };
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

  std::shared_ptr<countable_parameter> get_by_name(const std::string &name) {
    for (auto p : *this) {
      if (p->get_name().compare(name) == 0) {
        return p;
      }
    }
    return nullptr;
  }

  template <typename T> void add_parameter(T &p) {
    std::shared_ptr<countable_parameter_wrapper<T>> cloned =
        std::make_shared<countable_parameter_wrapper<T>>(p);
    this->push_back(cloned);
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
