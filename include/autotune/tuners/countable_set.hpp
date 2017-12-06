#pragma once

#include "../abstract_parameter.hpp"

namespace autotune {

class countable_set {

  class countable_parameter {
  public:
    virtual bool next() = 0;
    virtual bool prev() = 0;
    virtual size_t count_values() const = 0;
    virtual const std::string &get_name() const = 0;
    virtual const std::string get_value() const = 0;
    virtual std::string to_parameter_source_line() = 0;
    virtual void reset() = 0;
    virtual std::shared_ptr<abstract_parameter> clone() = 0;
  };

  // interface-wrapper generating template
  template <typename T>
  class countable_parameter_wrapper : public countable_parameter {
    const std::shared_ptr<T> p;

  public:
    countable_parameter_wrapper(const std::shared_ptr<T> p) : p(p) {}

    virtual bool next() override { return p->next(); };
    virtual bool prev() override { return p->prev(); };
    virtual size_t count_values() override { return p->count_values(); };
    virtual const std::string &get_name() const override {
      return p->get_name();
    };
    virtual const std::string get_value() const override {
      return p->get_value();
    };
    virtual std::string to_parameter_source_line() override {
      return p->to_parameter_source_line();
    };
    virtual void reset() override { p->reset(); };
    virtual std::shared_ptr<abstract_parameter> clone() override {
      return p->clone();
    };
  };

  std::vector<std::shared_ptr<countable_parameter>> countable_parameters;

public:
  template <typename T> void add_countable_parameter(std::shared_ptr<T> p) {
    countable_parameters.emplace_back(
        std::make_shared<countable_parameter_wrapper>(p));
  }
};
}
