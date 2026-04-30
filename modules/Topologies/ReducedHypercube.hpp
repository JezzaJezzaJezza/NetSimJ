#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include "Base.hpp"

namespace topo {

  using BitMask = std::uint64_t;

  class ReducedHypercube : public BaseTopo<ReducedHypercube, BitMask> {
  private:
    const std::size_t k_field;
    const std::size_t n_sub;
    std::size_t v_dim;
    std::size_t num_nodes;

    std::size_t subfield_m(BitMask x) const {
      std::size_t shift = k_field - n_sub;
      BitMask sub = (x >> shift) & ((BitMask{1} << n_sub) - 1);
      return static_cast<std::size_t>(sub);
    }

    BitMask cross_neighbour(BitMask x) const {
      std::size_t m = subfield_m(x);
      std::size_t dim_cross = k_field + m;
      BitMask mask = BitMask{1} << dim_cross;
      return x ^ mask;
    }

  public:
    using node_type = BitMask;

    explicit ReducedHypercube(std::size_t k, std::size_t n) : k_field(k), n_sub(n) {
      if (n == 0) {
        throw std::invalid_argument("ReducedHypercube: n must be > 0");
      }
      if (k < n) {
        throw std::invalid_argument("ReducedHypercube: require k >= n");
      }
      if (n >= sizeof(BitMask) * 8) {
        throw std::runtime_error("ReducedHypercube: n too large for BitMask");
      }
      v_dim = k + (std::size_t{1} << n);
      if (v_dim > sizeof(BitMask) * 8) {
        throw std::runtime_error("ReducedHypercube: dimension too large for BitMask");
      }
      num_nodes = node_count_impl();
      if (num_nodes == 0) {
        throw std::runtime_error("ReducedHypercube: node_count overflow");
      }
    }

    std::size_t node_count_impl() const {
      return std::size_t{1} << v_dim;
    }

    template <typename F>
    void for_each_node_impl(F&& f) const {
      for (std::size_t i = 0; i < num_nodes; i++) {
        f(static_cast<BitMask>(i));
      }
    }

    template <typename F>
    void for_each_neighbour_impl(const BitMask& x, F&& f) const {
      for (std::size_t d = 0; d < k_field; d++) {
        BitMask mask = BitMask{1} << d;
        f(x ^ mask);
      }
      f(cross_neighbour(x));
    }

    std::size_t degree_impl(const BitMask&) const {
      return k_field + 1;
    }

    BitMask neighbour_at_impl(const BitMask& x, std::size_t i) const {
      if (i < k_field) {
        BitMask mask = BitMask{1} << i;
        return x ^ mask;
      } else if (i == k_field) {
        return cross_neighbour(x);
      }
      throw std::out_of_range("ReducedHypercube: neighbour index out of range");
    }
  };
}
