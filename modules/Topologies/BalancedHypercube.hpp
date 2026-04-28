#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include "Base.hpp"

namespace topo {

  struct BHNode {
    static constexpr std::size_t MAX_N = 16;

    std::uint8_t a[MAX_N];
  };

  inline bool operator==(const BHNode& x, const BHNode& y) {
    for (std::size_t i = 0; i < BHNode::MAX_N; i++) {
      if (x.a[i] != y.a[i]) return false;
    }
    return true;
  }

  inline bool operator!=(const BHNode& x, const BHNode& y) {
    return !(x == y);
  }

  class BalancedHypercube : public BaseTopo<BalancedHypercube, BHNode> {
  private:
    const std::size_t n;
    const std::size_t num_nodes;

    static std::size_t pow4(std::size_t e) {
      std::size_t r = 1;
      for (std::size_t i = 0; i < e; i++) {
        r *= 4;
      }
      return r;
    }

    void increment(BHNode& x) const {
      for (std::size_t i = 0; i < n; i++) {
        std::uint8_t v = x.a[i];
        if (v < 3) {
          x.a[i] = static_cast<std::uint8_t>(v + 1);
          return;
        } else {
          x.a[i] = 0;
        }
      }
    }

  public:
    using node_type = BHNode;

    explicit BalancedHypercube(std::size_t dim)
      : n(dim),
        num_nodes(pow4(dim)) {

      if (n == 0) {
        throw std::invalid_argument("BalancedHypercube: dimension must be > 0");
      }
      if (n > BHNode::MAX_N) {
        throw std::invalid_argument("BalancedHypercube: n > BHNode::MAX_N");
      }
    }

    std::size_t node_count_impl() const {
      return num_nodes;
    }

    template <typename F>
    void for_each_node_impl(F&& f) const {
      BHNode x{};
      for (std::size_t i = 0; i < num_nodes; i++) {
        f(x);
        increment(x);
      }
    }

    template <typename F>
    void for_each_neighbour_impl(const BHNode& x, F&& f) const {
      std::uint8_t a0 = x.a[0];
      std::uint8_t a0p = static_cast<std::uint8_t>((a0 + 1) % 4);
      std::uint8_t a0m = static_cast<std::uint8_t>((a0 + 3) % 4);

      {
        BHNode y = x;
        y.a[0] = a0p;
        f(y);
      }
      {
        BHNode y = x;
        y.a[0] = a0m;
        f(y);
      }

      std::uint8_t delta4 = (a0 % 2 == 0) ? 1u : 3u;

      for (std::size_t i = 1; i < n; i++) {
        std::uint8_t ai = x.a[i];
        std::uint8_t new_ai = static_cast<std::uint8_t>((ai + delta4) % 4);

        {
          BHNode y = x;
          y.a[i] = new_ai;
          y.a[0] = a0p;
          f(y);
        }

        {
          BHNode y = x;
          y.a[i] = new_ai;
          y.a[0] = a0m;
          f(y);
        }
      }
    }

    std::size_t degree_impl(const BHNode&) const {
      return 2 * n;
    }

    BHNode neighbour_at_impl(const BHNode& x, std::size_t i) const {
      const std::size_t deg = degree_impl(x);
      if (i >= deg) {
        throw std::out_of_range("BalancedHypercube: neighbour index out of range");
      }

      std::uint8_t a0 = x.a[0];
      std::uint8_t a0p = static_cast<std::uint8_t>((a0 + 1) % 4);
      std::uint8_t a0m = static_cast<std::uint8_t>((a0 + 3) % 4);
      std::uint8_t delta4 = (a0 % 2 == 0) ? 1u : 3u;

      if (i == 0) {
        BHNode y = x;
        y.a[0] = a0p;
        return y;
      }
      if (i == 1) {
        BHNode y = x;
        y.a[0] = a0m;
        return y;
      }

      std::size_t idx = i - 2;
      std::size_t dim = 1 + (idx / 2);
      bool use_plus = (idx % 2 == 0);

      std::uint8_t ai = x.a[dim];
      std::uint8_t new_ai = static_cast<std::uint8_t>((ai + delta4) % 4);

      BHNode y = x;
      y.a[dim] = new_ai;
      y.a[0] = use_plus ? a0p : a0m;
      return y;
    }

    std::size_t dim_count() const {
      return n;
    }

    bool dim_aligned(const BHNode& a, const BHNode& b, std::size_t dim) const {
      if (dim >= n) {
        throw std::out_of_range("BalancedHypercube: dim_aligned dim out of range");
      }
      return a.a[dim] == b.a[dim];
    }

    BHNode move_to(const BHNode& from, const BHNode& to, std::size_t dim) const {
      if (dim >= n) {
        throw std::out_of_range("BalancedHypercube: move_to dim out of range");
      }

      if (from.a[dim] == to.a[dim]) {
        return from;
      }

      if (dim == 0) {
        return neighbour_at_impl(from, 0);
      } else {
        std::size_t idx = 2 + 2 * (dim - 1);
        return neighbour_at_impl(from, idx);
      }
    }
  };
}
