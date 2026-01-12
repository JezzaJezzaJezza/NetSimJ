#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <cmath>
#include "Base.hpp"

namespace topo {
  
  using BitMask = std::uint64_t;  

  class Zcube : public BaseTopo<Zcube, BitMask> {
  private:
    const std::size_t n;
    const std::size_t num_nodes;

    // k(m) as in Zhu's definition
    static std::size_t kappa(std::size_t m) {
      if (m == 1) return 0;
      double md = static_cast<double>(m);
      double v = std::log2(md) - 2.0 * std::log2(std::log2(md));
      if (v < 1.0) return 1;
      return static_cast<std::size_t>(std::ceil(v));
    }

    BitMask phi(BitMask x, std::size_t m) const {
      std::size_t k = kappa(m);
      if (k == 0) return x;

      BitMask result = x;

      for (std::size_t t = 0; t < k; ++t) {
        std::size_t idx_top = m - 1 - t;
        std::size_t idx_bottom = k - 1 - t;

        BitMask top_bit = (x >> idx_top)    & BitMask{1};
        BitMask bottom_bit = (x >> idx_bottom) & BitMask{1};
        BitMask new_bit = top_bit ^ bottom_bit;

        // clear and set top bit
        result &= ~(BitMask{1} << idx_top);
        result |=  (new_bit     << idx_top);
      }
      return result;
    }

    BitMask neighbour_dim(BitMask x,
                          std::size_t dim,
                          std::size_t cur_n) const {
      if (cur_n == 1) {
        if (dim != 0) {
          throw std::out_of_range("Zcube: bad dim for cur_n=1");
        }
        return x ^ BitMask{1};  // K2
      }

      const std::size_t msb_idx = cur_n - 1;
      const BitMask low_mask = (BitMask{1} << msb_idx) - 1;

      BitMask prefix = (x >> msb_idx) & BitMask{1};
      BitMask low    = x & low_mask;

      if (dim == msb_idx) {
        BitMask mapped = phi(low, msb_idx);
        BitMask other_prefix = prefix ^ BitMask{1};
        return (other_prefix << msb_idx) | mapped;
      } else {
        BitMask low_nbr = neighbour_dim(low, dim, cur_n - 1);
        return (prefix << msb_idx) | low_nbr;
      }
    }

  public:
    using node_type = BitMask;
      
    explicit Zcube(std::size_t dim)
      : n(dim),
        num_nodes(node_count_impl()) {
      if (n == 0) {
        throw std::runtime_error("Zcube requires dimension > 0");
      }
      if (n > sizeof(BitMask) * 8) {
        throw std::runtime_error("Zcube: dimension too large for BitMask type");
      }
    }

    std::size_t node_count_impl() const {
      return std::size_t{1} << n;
    }

    template <typename F>
    void for_each_node_impl(F&& f) const {
      for (std::size_t i = 0; i < num_nodes; ++i) {
        BitMask x = static_cast<BitMask>(i);
        f(x);
      }
    }

    template <typename F>
    void for_each_neighbour_impl(const BitMask& x, F&& f) const {
      for (std::size_t dim = 0; dim < n; ++dim) {
        f(neighbour_at_impl(x, dim));
      }
    }

    std::size_t degree_impl(const BitMask&) const {
      return n;
    }

    BitMask neighbour_at_impl(const BitMask& x, std::size_t i) const {
      if (i >= n) {
        throw std::out_of_range("Zcube: neighbour index out of range");
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
  };
}
