#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include "Base.hpp"

namespace topo {

  using BitMask = std::uint64_t;

  class Mobiuscube: public BaseTopo<Mobiuscube, BitMask> {
    private:
      const std::size_t n;
      const std::size_t num_nodes;
      const BitMask mask_n;

      BitMask mobius_neighbour(BitMask x, std::size_t dim) const {
        if (dim >= n) {
          throw std::out_of_range("Mcube: dim out of range");
        }

        bool prev_bit = (dim == 0) ? true : ((x >> (dim - 1)) & 1);
        BitMask flip = prev_bit ? (mask_n & (~BitMask{0} << dim))
                                : (BitMask{1} << dim);
        return x ^ flip;
      }

    public:
      using node_type = BitMask;

      explicit Mobiuscube(std::size_t dim) : n(dim), num_nodes(node_count_impl()), mask_n(n >= sizeof(BitMask) * 8 ? ~BitMask{0} : (BitMask{1} << n) - 1) {
        if (n == 0) {
          throw std::invalid_argument("Mcube: dimension must be > 0");
        }
        if (n > sizeof(BitMask) * 8) {
          throw std::runtime_error("Mcube: dimension too large for BitMask");
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
      void for_each_endpoint_impl(F&& f) const {
        for_each_node_impl(std::forward<F>(f));
      }

      template <typename F>
      void for_each_neighbour_impl(const BitMask& x, F&& f) const {
        for (std::size_t dim = 0; dim < n; dim++) {
          f(mobius_neighbour(x, dim));
        }
      }

      std::size_t degree_impl(const BitMask&) const {
        return n;
      }

      BitMask neighbour_at_impl(const BitMask& x, std::size_t i) const {
        if (i >= n) {
          throw std::out_of_range("Mcube: neighbour index out of range");
        }
        return mobius_neighbour(x, i);
      }

      std::size_t dim_count() const {
        return n;
      }

      bool dim_aligned(BitMask a, BitMask b, std::size_t dim) const {
        if (dim >= n) {
          throw std::out_of_range("Mcube: dim_aligned dim out of range");
        }
        BitMask mask = BitMask{1} << dim;
        return ((a ^ b) & mask) == 0;
      }

      BitMask move_to(BitMask from, BitMask /*to*/, std::size_t dim) const {
        if (dim >= n) {
          throw std::out_of_range("Mcube: move_to dim out of range");
        }
        return mobius_neighbour(from, dim);
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
