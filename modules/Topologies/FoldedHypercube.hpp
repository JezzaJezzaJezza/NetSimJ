#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include "Base.hpp"

namespace topo {
  
  using BitMask = std::uint64_t;  

  class FoldedHypercube : public BaseTopo<FoldedHypercube, BitMask> {
  private:
    const std::size_t n;
    const std::size_t num_nodes;

    BitMask full_mask() const {
      const std::size_t bits = sizeof(BitMask) * 8;
      if (n >= bits) {
        return ~BitMask{0};
      }
      return (BitMask{1} << n) - 1;
    }

  public:
    using node_type = BitMask;
      
    explicit FoldedHypercube(std::size_t dim)
      : n(dim),
        num_nodes(node_count_impl()) {
      if (n == 0) {
        throw std::runtime_error("FoldedHypercube requires dimension > 0");
      }
      if (n > sizeof(BitMask) * 8) {
        throw std::runtime_error("Dimension too large for BitMask type");
      }
    }

    std::size_t node_count_impl() const {
      return std::size_t{1} << n;
    }

    template <typename F>
    void for_each_node_impl(F&& f) const {
      for (std::size_t i = 0; i < num_nodes; i++) {
        BitMask x = static_cast<BitMask>(i);
        f(x);
      }
    }

    template <typename F>
    void for_each_neighbour_impl(const BitMask& x, F&& f) const {
      // Hypercube neighbours: Hamming distance 1
      for (std::size_t i = 0; i < n; i++) {
        BitMask mask = BitMask{1} << i;
        BitMask neighbour = x ^ mask;
        f(neighbour);
      }

      // Fold edge: connect to bitwise complement within n bits
      BitMask comp = (~x) & full_mask();
      f(comp);
    }

    std::size_t degree_impl(const BitMask&) const {
      return n + 1;
    }

    BitMask neighbour_at_impl(const BitMask& x, std::size_t i) const {
      if (i < n) {
        BitMask mask = BitMask{1} << i;
        return x ^ mask;
      }
      if (i == n) {
        BitMask comp = (~x) & full_mask();
        return comp;
      }
      throw std::out_of_range("FoldedHypercube: neighbour index out of range");
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
      if (((from ^ to) & mask) != 0) {
        return from ^ mask;
      }
      return from;
    }
  };
}
