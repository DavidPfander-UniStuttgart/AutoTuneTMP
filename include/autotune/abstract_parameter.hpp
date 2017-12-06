#pragma once

// an abstract parameter should never get instantiated
// only occurs as part of construction of the individual parameter types

namespace autotune {

class abstract_parameter;

class abstract_parameter {
protected:
  std::string name;
  abstract_parameter(const std::string &name) : name(name){};

public:
  const std::string &get_name() const { return this->name; }
  virtual const std::string get_value() const = 0;
  virtual std::string to_parameter_source_line() = 0;
  virtual void reset() = 0;
  virtual std::shared_ptr<abstract_parameter> clone() = 0;
};

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
