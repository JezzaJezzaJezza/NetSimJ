#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include "Base.hpp"

namespace topo {

  using BitMask = std::uint64_t;

  class ReducedHypercube : public BaseTopo<ReducedHypercube, BitMask> {
  private:
    const std::size_t k_field;    // k in RH(k, n)
    const std::size_t n_sub;      // n in RH(k, n)
    const std::size_t v_dim;      // v = k + 2^n, dimension of underlying hypercube
    const std::size_t num_nodes;  // 2^v

    // helper: (1 << n_sub) as size_t, with basic sanity check done in ctor
    static std::size_t pow2(std::size_t e) {
      return std::size_t{1} << e;
    }

    // compute m = value of 0th subfield (n_sub MSBs of 0th field)
    // 0th field = k_field LSBs = bits [0 .. k_field-1]
    // 0th subfield = n_sub MSBs of that = bits [k_field-1 .. k_field-n_sub]
    std::size_t subfield_m(BitMask x) const {
      // shift so that bit (k_field - n_sub) becomes bit 0
      std::size_t shift = k_field - n_sub;
      BitMask sub = (x >> shift) & ((BitMask{1} << n_sub) - 1);
      return static_cast<std::size_t>(sub);
    }

    // The unique "set 2" neighbour (cross-BB edge)
    BitMask cross_neighbour(BitMask x) const {
      std::size_t m = subfield_m(x); // 0 <= m < 2^n_sub
      std::size_t dim_cross = k_field + m; // bit index in 1st field
      BitMask mask = BitMask{1} << dim_cross;
      return x ^ mask;
    }

  public:
    using node_type = BitMask;

    // RH(k, n): k_field = k, n_sub = n
    explicit ReducedHypercube(std::size_t k, std::size_t n)
      : k_field(k),
        n_sub(n),
        v_dim([&]{
          if (n == 0) {
            throw std::invalid_argument("ReducedHypercube: n must be > 0");
          }
          if (k < n) {
            throw std::invalid_argument("ReducedHypercube: require k >= n");
          }
          // guard 1 << n
          if (n >= sizeof(BitMask) * 8) {
            throw std::runtime_error("ReducedHypercube: n too large for BitMask");
          }
          std::size_t high = pow2(n); // 2^n bits in 1st field
          std::size_t v = k + high;   // total dimension
          if (v > sizeof(BitMask) * 8) {
            throw std::runtime_error("ReducedHypercube: dimension too large for BitMask");
          }
          return v;
        }()),
        num_nodes(node_count_impl())
    {
      if (num_nodes == 0) {
        throw std::runtime_error("ReducedHypercube: node_count overflow");
      }
    }

    // Underlying hypercube dimension v = k + 2^n
    std::size_t node_count_impl() const {
      return std::size_t{1} << v_dim;
    }

    template <typename F>
    void for_each_node_impl(F&& f) const {
      for (std::size_t i = 0; i < num_nodes; ++i) {
        BitMask x = static_cast<BitMask>(i);
        f(x);
      }
    }

    // Neighbours of x in RH(k, n):
    // - Set 1: flip bits 0..k_field-1 (local k-cube inside BB)
    // - Set 2: one cross edge via cross_neighbour(x)
    template <typename F>
    void for_each_neighbour_impl(const BitMask& x, F&& f) const {
      // Set 1
      for (std::size_t d = 0; d < k_field; ++d) {
        BitMask mask = BitMask{1} << d;
        BitMask nbr = x ^ mask;
        f(nbr);
      }
      // Set 2
      BitMask cross = cross_neighbour(x);
      f(cross);
    }

    // Degree = k + 1 (k local edges + 1 cross edge)
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

    // NOTE:
    // We do NOT define dim_count / dim_aligned / move_to here on purpose.
    // The reduced hypercube is built by *removing* many hypercube dimensions,
    // so the global “dimension-order routing” abstraction you used for Hypercube
    // does not carry over cleanly. Any DOR that assumes one edge per bit at
    // every node will be lying about the actual RH edges.
    //
    // If you *really* want to hack a DOR-ish thing for RH, do it in a separate
    // router that works off for_each_neighbour and understands the two-level
    // (BB/SB) structure instead of pretending it’s a full Q_v.
  };
}
