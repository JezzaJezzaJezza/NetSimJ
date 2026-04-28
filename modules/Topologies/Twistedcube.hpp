#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include "Base.hpp"

namespace topo {

  using BitMask = std::uint64_t;

  class TwistedCube : public BaseTopo<TwistedCube, BitMask> {
  private:
    const std::size_t n;
    const std::size_t num_nodes;

    static constexpr std::size_t MAX_BITS = 63;

    static std::size_t pow2(std::size_t e) {
      if (e >= MAX_BITS) {
        throw std::runtime_error("TwistedCube: dimension too large for BitMask");
      }
      return std::size_t{1} << e;
    }

    static bool prefix_parity(BitMask x, std::size_t k) {
      bool p = false;
      for (std::size_t i = 0; i <= k; i++) {
        p ^= ((x >> i) & BitMask{1}) != 0;
      }
      return p;
    }

    BitMask neighbour_at_rec(BitMask x, std::size_t dim, std::size_t cur_n) const {
      if (cur_n == 1) {
        if (dim != 0) {
          throw std::out_of_range("TwistedCube: dim out of range in TQ1");
        }
        return x ^ BitMask{1};
      }

      if (dim >= cur_n) {
        throw std::out_of_range("TwistedCube: dim >= cur_n");
      }

      if (dim < cur_n - 2) {
        BitMask low_mask = (BitMask{1} << (cur_n - 2)) - 1;
        BitMask low = x & low_mask;
        BitMask high = x & ~low_mask;

        BitMask low_nb = neighbour_at_rec(low, dim, cur_n - 2);
        return high | low_nb;
      }

      const std::size_t k = cur_n - 3;
      bool p = prefix_parity(x, k);

      BitMask bit_n1 = BitMask{1} << (cur_n - 1);
      BitMask bit_n2 = BitMask{1} << (cur_n - 2);

      if (dim == cur_n - 1) {
        return x ^ bit_n1;
      }

      if (!p) {
        return x ^ (bit_n1 | bit_n2);
      } else {
        return x ^ bit_n2;
      }
    }

  public:
    using node_type = BitMask;

    explicit TwistedCube(std::size_t dim)
      : n(dim),
        num_nodes(pow2(dim)) {

      if (n == 0) {
        throw std::invalid_argument("TwistedCube: dimension must be > 0");
      }
      if (n > MAX_BITS) {
        throw std::invalid_argument("TwistedCube: dimension too large for BitMask");
      }
      if (n % 2 == 0) {
        throw std::invalid_argument("TwistedCube: n must be odd (1,3,5,...)");
      }
    }

    std::size_t node_count_impl() const {
      return num_nodes;
    }

    template <typename F>
    void for_each_node_impl(F&& f) const {
      for (std::size_t i = 0; i < num_nodes; i++) {
        BitMask x = static_cast<BitMask>(i);
        f(x);
      }
    }

    template <typename F>
    void for_each_endpoint_impl(F&& f) const {
      for_each_node_impl(std::forward<F>(f));
    }

    template <typename F>
    void for_each_neighbour_impl(const BitMask& x, F&& f) const {
      for (std::size_t dim = 0; dim < n; dim++) {
        BitMask nb = neighbour_at_impl(x, dim);
        f(nb);
      }
    }

    std::size_t degree_impl(const BitMask&) const {
      return n;
    }

    BitMask neighbour_at_impl(const BitMask& x, std::size_t i) const {
      if (i >= n) {
        throw std::out_of_range("TwistedCube: neighbour index out of range");
      }
      return neighbour_at_rec(x, i, n);
    }

    std::size_t dim_count() const {
      return n;
    }

    bool dim_aligned(BitMask a, BitMask b, std::size_t dim) const {
      if (dim >= n) {
        throw std::out_of_range("TwistedCube: dim_aligned dim out of range");
      }
      BitMask mask = BitMask{1} << dim;
      return ((a ^ b) & mask) == 0;
    }

    BitMask move_to(BitMask from, BitMask to, std::size_t dim) const {
      if (dim >= n) {
        throw std::out_of_range("TwistedCube: move_to dim out of range");
      }
      if (dim_aligned(from, to, dim)) {
        return from;
      }
      return neighbour_at_impl(from, dim);
    }

    std::string node_to_string_impl(BitMask x) const {
      std::string s;
      s.reserve(n);
      for (int i = static_cast<int>(n) - 1; i >= 0; i--) {
        s.push_back((x >> i) & 1 ? '1' : '0');
      }
      return s;
    }
  };
}
