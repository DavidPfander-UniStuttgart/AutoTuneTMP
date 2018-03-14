#pragma once

namespace autotune {

template <typename R, typename... Args> class with_tests {
private:
  std::function<bool(R)> t;

public:
  void setup_test(std::function<bool(R)> t_) { t = t_; };

  bool has_test() { return t ? true : false; }

  bool test(R r) { return t(r); };
};

template <typename R, typename... Args> class without_tests {};
}
