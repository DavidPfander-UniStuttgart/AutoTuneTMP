#pragma once

namespace autotune {

template <typename T> class randomizable_parameter_wrapper;

class randomizable_parameter {
public:
  virtual const std::string &get_name() const = 0;
  virtual void set_random_value() = 0;
  virtual const std::string get_value() const = 0;
  virtual std::shared_ptr<randomizable_parameter> clone_wrapper() = 0;
  template <typename T> T &get_unwrapped_parameter() {
    auto derived = dynamic_cast<randomizable_parameter_wrapper<T> *>(this);
    return derived->unwrapped_parameter();
  }
};

// interface-wrapper generating template
template <typename T>
class randomizable_parameter_wrapper : public randomizable_parameter {
  T p;

public:
  randomizable_parameter_wrapper(T p) : p(p) {}

  randomizable_parameter_wrapper(const randomizable_parameter_wrapper<T> &other)
      : p(other.p) {}

  virtual const std::string &get_name() const override { return p.get_name(); }

  virtual void set_random_value() override { return p.set_random_value(); }
  virtual const std::string get_value() const override { return p.get_value(); }
  virtual std::shared_ptr<randomizable_parameter> clone_wrapper() override {
    return std::make_shared<randomizable_parameter_wrapper<T>>(*this);
  }
  T &unwrapped_parameter() { return p; }
};

class randomizable_set
    : public std::vector<std::shared_ptr<randomizable_parameter>> {

public:
  randomizable_set() : std::vector<std::shared_ptr<randomizable_parameter>>() {}

  randomizable_set(const randomizable_set &other)
      : std::vector<std::shared_ptr<randomizable_parameter>>() {
    for (size_t i = 0; i < other.size(); i++) {
      this->push_back(other[i]->clone_wrapper());
    }
  }

  randomizable_set &operator=(const randomizable_set &other) {
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
    std::shared_ptr<randomizable_parameter_wrapper<T>> cloned =
        std::make_shared<randomizable_parameter_wrapper<T>>(p);
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
