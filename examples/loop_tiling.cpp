#include <iostream>
#include <memory>

class trivial_iterator {
private:
  size_t start;
  size_t stop;
  size_t step;
  size_t cur;

public:
  trivial_iterator(size_t start, size_t stop, size_t step)
      : start(start), stop(stop), step(step), cur(start) {}
  bool has_next() { return cur < stop; }
  size_t next() {
    size_t tmp = cur;
    cur += step;
    return tmp;
  }
  void reset(size_t new_start, size_t new_stop) {
    start = new_start;
    stop = new_stop;
  }
};

class tiled_iterator {
private:
  size_t start;
  size_t stop;
  size_t step;
  size_t cur;
  std::unique_ptr<tiled_iterator> inner;

public:
  tiled_iterator(size_t start, size_t stop, size_t step)
      : start(start), stop(stop), cur(start), step(step), inner(nullptr) {}
  tiled_iterator(const tiled_iterator &) = delete;
  tiled_iterator(tiled_iterator &) = delete;
  tiled_iterator(tiled_iterator &&other) = delete;
  // : start(other.start), stop(other.stop), step(other.step), cur(other.cur),
  //   inner(std::move(other.inner)){};
  tiled_iterator(size_t step)
      : start(0), stop(0), cur(start), step(step), inner(nullptr) {}

  bool has_next() {
    // std::cout << "start: " << start << " stop: " << stop << " step: " << step
    //           << " cur:" << cur << " ptr: " << this << std::endl;
    return cur < stop;
  }

  size_t next() {
    if (inner) {
      inner->reset(cur, cur + step);
    }
    size_t tmp = cur;
    cur += step;
    return tmp;
  }

  tiled_iterator &tile(size_t tile_step) {
    if (step % tile_step != 0) {
      throw;
    }

    inner = std::make_unique<tiled_iterator>(tile_step);
    return *inner;
  }

  void reset() { cur = start; } // for outer iterators

  void reset(size_t new_start, size_t new_stop) { // for inner iterator
    start = new_start;
    stop = new_stop;
    cur = new_start;
    // std::cout << "resetting myself to: " << start << " -> " << stop
    //           << " step: " << step << " ptr: " << this << std::endl;
  }

  bool is_tiled() { return inner != nullptr; }

  tiled_iterator &get_inner() { return *inner; }
};

template <size_t cur, size_t start, size_t stop, class F, class... T>
void apply_rec(std::tuple<T &...> &ts, F f) {
  if constexpr (cur < stop) {
    // std::cout << "apply cur: " << cur << std::endl;
    f(std::get<cur>(ts));
    apply_rec<cur + 1, start, stop, F, T...>(ts, f);
  }
}

template <size_t start, size_t stop, class F, class... T>
void apply(std::tuple<T &...> &its, F f) {
  apply_rec<start, start, stop>(its, f);
}

template <typename... Ts> class loop_index {

  std::tuple<Ts &...> iterators;
  constexpr static size_t N = sizeof...(Ts);
  std::array<size_t, N> indices;

  template <size_t cur, class F> void iterate_rec(F f) {
    if constexpr (cur < N) {
      auto &it = std::get<cur>(iterators);
      while (it.has_next()) {
        indices[cur] = it.next();
        // reset all children
        apply<cur + 1, N>(iterators, [](tiled_iterator &it) { it.reset(); });
        iterate_rec<cur + 1>(f);
      }
    } else {
      f(indices);
    }
  };

public:
  loop_index(Ts &... tiled_iterator) : iterators(tiled_iterator...) {}

  template <class F> void iterate(F f) { iterate_rec<0>(f); };
};

int main(void) {
  tiled_iterator l0(0, 8, 4);
  tiled_iterator l1(0, 4, 2);
  // tiled_iterator l3(0, 12, 4);
  tiled_iterator &l0_p = l0.tile(2);
  tiled_iterator &l0_pp = l0_p.tile(1);
  tiled_iterator &l1_p = l1.tile(1);
  // while (l0.has_next()) {
  //   size_t value = l0.next();
  //   while (l0_p.has_next()) {
  //     size_t value_p = l0_p.next();
  //     while (l0_pp.has_next()) {
  //       size_t value_pp = l0_pp.next();
  //       std::cout << "value_pp: " << value_pp << ", value_p:" << value_p
  //                 << ", value: " << value << std::endl;
  //     }
  //   }
  // }

  loop_index li(l0, l1, l0_p, l0_pp, l1_p);
  li.iterate([](std::array<size_t, 5> &indices) {
    std::cout << "i0: " << indices[0] << ", i1:" << indices[1]
              << ", i2: " << indices[2] << ", i3: " << indices[3]
              << ", i4: " << indices[4] << std::endl;
  });

  return EXIT_SUCCESS;
}
