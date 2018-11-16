#include <memory>

namespace opttmp {
namespace loop {
// class trivial_iterator {
// private:
//   size_t start;
//   size_t stop;
//   size_t step;
//   size_t cur;

// public:
//   trivial_iterator(size_t start, size_t stop, size_t step)
//       : start(start), stop(stop), step(step), cur(start) {}
//   bool has_next() { return cur < stop; }
//   size_t next() {
//     size_t tmp = cur;
//     cur += step;
//     return tmp;
//   }
//   void reset(size_t new_start, size_t new_stop) {
//     start = new_start;
//     stop = new_stop;
//   }
// };

class tiled_iterator {
private:
  size_t start;
  size_t stop;
  size_t step;
  size_t cur;
  std::unique_ptr<tiled_iterator> inner;

public:
  tiled_iterator(size_t start, size_t stop, size_t step)
      : start(start), stop(stop), step(step), cur(start), inner(nullptr) {}
  tiled_iterator(const tiled_iterator &) = delete;
  tiled_iterator(tiled_iterator &) = delete;
  tiled_iterator(tiled_iterator &&other) = delete;
  // : start(other.start), stop(other.stop), step(other.step), cur(other.cur),
  //   inner(std::move(other.inner)){};
  tiled_iterator(size_t step)
      : start(0), stop(0), step(step), cur(start), inner(nullptr) {}

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
  }

public:
  loop_index(Ts &... tiled_iterator) : iterators(tiled_iterator...) {}

  template <class F> void iterate(F f) { iterate_rec<0>(f); }
};
} // namespace loop
} // namespace opttmp
