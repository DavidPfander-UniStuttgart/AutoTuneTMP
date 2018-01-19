#pragma once

#include <memory>
#include <map>

// an abstract parameter should never get instantiated
// only occurs as part of construction of the individual parameter types

namespace autotune {

class abstract_parameter {
  const std::vector<std::string> flags;
public:
  abstract_parameter(const std::map<std::string, bool>& flags) : flags(flags) {};
  virtual bool has_flag(const std::string& flag) const {
    auto it = flags.find(flag);
    if (it != flags.end()) return it->second;
    return false;
  }
  virtual const std::string &get_name() const = 0;
  virtual const std::string get_value() const = 0;
  virtual void set_initial() = 0;
  virtual std::shared_ptr<abstract_parameter> clone_wrapper() = 0;
};

template <typename T>
class abstract_parameter_wrapper : public abstract_parameter {
  T p;
public:
  abstract_parameter_wrapper(T p) : p(p) {}
  abstract_parameter_wrapper(const abstract_parameter_wrapper<T> &other) : p(other.p) {}
  virtual const std::string &get_name() const override { return p.get_name(); }
  virtual const std::string get_value() const override { return p.get_value(); }
  virtual void set_initial() override { p.set_initial(); };
  virtual std::shared_ptr<abstract_parameter> clone_wrapper() override {
    return std::make_shared<abstract_parameter_wrapper<T>>(*this);
  }
};

// class abstract_parameter;

// class abstract_parameter {
// protected:
//   std::string name;
//   abstract_parameter(const std::string &name) : name(name){};

// public:
//   const std::string &get_name() const { return this->name; }
//   virtual const std::string get_value() const = 0;
//   // virtual std::string to_parameter_source_line() = 0;
//   // virtual void set_initial() = 0;
//   // virtual std::shared_ptr<abstract_parameter> clone() = 0;
// };

// class step_parameter : public virtual abstract_parameter {
// public:
//   // step_parameter(const std::string &name) : abstract_parameter(name) {}

//   virtual bool next() = 0;

//   virtual bool prev() = 0;
// };

// class countable_parameter : public step_parameter {
// public:
//   // countable_parameter(const std::string &name) : step_parameter(name) {}

//   virtual size_t count_values() const = 0;
// };
}
