#pragma once

#include <memory>
#include <vector>

namespace autotune {

// an abstract parameter should never get instantiated
// only occurs as part of construction of the individual parameter types

class abstract_parameter;

class abstract_parameter {
protected:
  std::string name;
  abstract_parameter(const std::string &name) : name(name){};

public:
  const std::string &get_name() const { return this->name; }
  virtual const std::string &get_value() const = 0;
  virtual std::string to_parameter_source_line() = 0;
  virtual size_t count_values() const = 0;
  virtual void reset() = 0;
  virtual std::shared_ptr<abstract_parameter> clone() = 0;
};

class fixed_set_parameter : public abstract_parameter {
private:
  size_t cur_index;
  std::vector<std::string> values;

public:
  fixed_set_parameter(const std::string &name,
                      const std::vector<std::string> &values)
      : abstract_parameter(name), cur_index(0), values(values) {}

  const std::vector<std::string> &get_values() const { return this->values; }

  void set_index(size_t new_index) { cur_index = new_index; };

  const std::string &get_value(size_t index) const {
    return this->values[index];
  }

  virtual const std::string &get_value() const override {
    return this->values[cur_index];
  }

  virtual size_t count_values() const override { return values.size(); }

  virtual void reset() override {
    // TODO: should be extended, so that an initial guess can be supplied
    cur_index = 0;
  }

  virtual std::string to_parameter_source_line() override {
    return "#define " + name + " " + values[cur_index] + "\n";
  }

  // TODO: remove this overload?
  std::string to_parameter_source_line(size_t index) {
    return "#define " + name + " " + values[index] + "\n";
  }

  virtual std::shared_ptr<abstract_parameter> clone() override {
    std::shared_ptr<fixed_set_parameter> new_instance =
        std::make_shared<fixed_set_parameter>(this->name, this->values);
    return std::dynamic_pointer_cast<abstract_parameter>(new_instance);
  };
};

// using parameter_set = std::vector<std::shared_ptr<abstract_parameter>>;
class parameter_set : public std::vector<std::shared_ptr<abstract_parameter>> {
public:
  // void operator=(parameter_set const &other) {
  //   for (size_t i = 0; i < other.size(); i++) {
  //     this->push_back(other[i]);
  //   }
  // }

  parameter_set clone() {
    parameter_set new_instance;
    for (size_t i = 0; i < this->size(); i++) {
      new_instance.push_back(this->operator[](i)->clone());
    }
    return new_instance;
  }
};
}
