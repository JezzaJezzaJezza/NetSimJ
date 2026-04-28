#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include "Base.hpp"

namespace topo {

  using BitMask = std::uint64_t;

  class CrossedCube : public BaseTopo<CrossedCube, BitMask> {
  private:
    const std::size_t n;
    const std::size_t num_nodes;

    static inline void pair_map(unsigned hi, unsigned lo,
                                unsigned &out_hi, unsigned &out_lo) {
      unsigned p = (hi << 1) | lo;
      switch (p) {
        case 0b00: out_hi = 0; out_lo = 0; break;
        case 0b10: out_hi = 1; out_lo = 0; break;
        case 0b01: out_hi = 1; out_lo = 1; break;
        case 0b11: out_hi = 0; out_lo = 1; break;
        default:   out_hi = 0; out_lo = 0; break;
      }
    }

    BitMask cross_map_low(BitMask s, std::size_t cur_n) const {
      const std::size_t low_bits = cur_n - 1;
      BitMask t = 0;

      if ((cur_n & 1u) == 0) {
        std::size_t idx = low_bits - 1;
        unsigned b = (s >> idx) & 1u;
        t |= (BitMask{b} << idx);
      }

      const std::size_t pair_bound = low_bits / 2;
      for (std::size_t i = 0; i < pair_bound; i++) {
        std::size_t bit_lo = 2 * i;
        std::size_t bit_hi = 2 * i + 1;

        unsigned lo = (s >> bit_lo) & 1u;
        unsigned hi = (s >> bit_hi) & 1u;

        unsigned v_hi = 0, v_lo = 0;
        pair_map(hi, lo, v_hi, v_lo);

        t |= (BitMask{v_lo} << bit_lo);
        t |= (BitMask{v_hi} << bit_hi);
      }

      return t;
    }

    BitMask neighbour_dim(BitMask x, std::size_t dim, std::size_t cur_n) const {
      if (cur_n == 1) {
        if (dim != 0) {
          throw std::out_of_range("CrossedCube: bad dim in neighbour_dim");
        }
        return x ^ BitMask{1};
      }

      const std::size_t msb_idx = cur_n - 1;
      const BitMask low_mask = (BitMask{1} << msb_idx) - 1;

      unsigned prefix = (x >> msb_idx) & 1u;
      BitMask low = x & low_mask;

      if (dim == msb_idx) {
        BitMask mapped_low = cross_map_low(low, cur_n);
        unsigned other_prefix = 1u - prefix;
        return (BitMask{other_prefix} << msb_idx) | mapped_low;
      } else {
        BitMask low_nbr = neighbour_dim(low, dim, cur_n - 1);
        return (BitMask{prefix} << msb_idx) | low_nbr;
      }
    }

  public:
    using node_type = BitMask;

    explicit CrossedCube(std::size_t dim)
      : n(dim),
        num_nodes(node_count_impl()) {
      if (n == 0) {
        throw std::runtime_error("CrossedCube: dimension must be > 0");
      }
      if (n > sizeof(BitMask) * 8) {
        throw std::runtime_error("CrossedCube: dimension too large for BitMask");
      }
    }

    std::size_t node_count_impl() const {
      return std::size_t{1} << n;
    }

    template <typename F>
    void for_each_node_impl(F&& f) const {
      for (std::size_t i = 0; i < num_nodes; i++) {
        f(static_cast<BitMask>(i));
      }
    }

    template <typename F>
    void for_each_endpoint_impl(F&& f) const {
      for_each_node_impl(std::forward<F>(f));
    }

    template <typename F>
    void for_each_neighbour_impl(const BitMask& x, F&& f) const {
      for (std::size_t dim = 0; dim < n; dim++) {
        f(neighbour_at_impl(x, dim));
      }
    }

    std::size_t degree_impl(const BitMask&) const {
      return n;
    }

    BitMask neighbour_at_impl(const BitMask& x, std::size_t i) const {
      if (i >= n) {
        throw std::out_of_range("CrossedCube: neighbour index out of range");
      }
      return neighbour_dim(x, i, n);
    }

    std::size_t dim_count() const {
      return n;
    }

    bool dim_aligned(BitMask a, BitMask b, std::size_t dim) const {
      BitMask mask = BitMask{1} << dim;
      return ((a ^ b) & mask) == 0;
    }

    BitMask move_to(BitMask from, BitMask to, std::size_t dim) const {
      BitMask mask = BitMask{1} << dim;
      if (((from ^ to) & mask) == 0) {
        return from;
      }
      return neighbour_dim(from, dim, n);
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
