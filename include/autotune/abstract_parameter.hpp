#pragma once

#include <map>
#include <memory>

// an abstract parameter should never get instantiated
// only occurs as part of construction of the individual parameter types

namespace autotune {

// template <typename T> class abstract_parameter_wrapper;

class abstract_parameter {
public:
  virtual const std::string &get_name() const = 0;
  virtual const std::string get_value() const = 0;
  virtual void set_initial() = 0;
  // virtual std::shared_ptr<abstract_parameter> clone_wrapper() = 0;
  /*template<typename T> std::shared_ptr<T> clone_wrapper() {
    return std::make_shared<abstract_parameter_wrapper<T>>(*this);
  }*/

  // TODO: BUGGY!
  // template <typename T> T &get_unwrapped_parameter() {
  //   auto derived = dynamic_cast<abstract_parameter_wrapper<T> *>(this);
  //   return derived->unwrapped_parameter();
  // }
};

// template <typename T>
// class abstract_parameter_wrapper : public abstract_parameter {
//   T p;

// public:
//   abstract_parameter_wrapper(T p) : p(p) {}
//   abstract_parameter_wrapper(const abstract_parameter_wrapper<T> &other)
//       : p(other.p) {}
//   virtual const std::string &get_name() const override { return p.get_name(); }
//   virtual const std::string get_value() const override { return p.get_value(); }
//   virtual void set_initial() override { p.set_initial(); };
//   /*virtual std::shared_ptr<abstract_parameter> clone_wrapper() override {
//     return std::make_shared<abstract_parameter_wrapper<T>>(*this);
//   }*/
//   T &unwrapped_parameter() { return p; }
// };

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
