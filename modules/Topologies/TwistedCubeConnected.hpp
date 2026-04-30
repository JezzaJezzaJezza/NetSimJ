#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include "Base.hpp"


// The Twisted-Cube Connected Networks by WANG and ZHAO (recoined parity cube)
namespace topo {

  using BitMask = std::uint64_t;

  class TwistedCubeConnected : public BaseTopo<TwistedCubeConnected, BitMask> {
  private:
    const std::size_t n;
    const std::size_t num_nodes;

    static constexpr std::size_t MAX_BITS = sizeof(BitMask) * 8;

    static BitMask pow2(std::size_t n) {
      if (n >= MAX_BITS) {
        throw std::runtime_error("TwistedCubeConnected: dimension too large for BitMask");
      }
      return BitMask{1} << n;
    }

    static std::size_t q_n(std::size_t dim) {
      return (dim - 1) / 2;
    }

    BitMask cross_from_zero(BitMask u, std::size_t dim) const {
      BitMask v = u;

      v ^= BitMask{1} << (dim - 1);

      const std::size_t q = q_n(dim);

      for (std::size_t t = 1; t <= q; t++) {
        std::size_t idx_u_2t_minus1 = 2 * t - 2;
        std::size_t idx_e_2t = 2 * t - 1;
        if (idx_e_2t >= dim - 1) break;

        if ((u >> idx_u_2t_minus1) & BitMask{1}) {
          v ^= BitMask{1} << idx_e_2t;
        }
      }

      unsigned parity = 0;
      if (dim >= 4) {
        for (std::size_t t = 3; t <= dim - 1; t++) {
          std::size_t idx = t - 1;
          parity ^= static_cast<unsigned>((u >> idx) & BitMask{1});
        }
      }

      if (parity & 1u) {
        v ^= BitMask{1} << 1;
      }

      return v;
    }

    BitMask cross_from_one(BitMask v, std::size_t dim) const {
      const std::size_t q = q_n(dim);

      unsigned u_bits[MAX_BITS]{};

      u_bits[dim] = 0;

      for (std::size_t i = 3; i <= dim - 1; i++) {
        bool v_i = ((v >> (i - 1)) & BitMask{1}) != 0;

        if (i % 2 == 1) {
          u_bits[i] = v_i ? 1u : 0u;
        } else {
          if (i <= 2 * q) {
            unsigned u_im1 = u_bits[i - 1];
            u_bits[i] = static_cast<unsigned>(v_i ? 1u : 0u) ^ u_im1;
          } else {
            u_bits[i] = v_i ? 1u : 0u;
          }
        }
      }

      unsigned parity = 0;
      if (dim >= 4) {
        for (std::size_t i = 3; i <= dim - 1; i++) {
          parity ^= u_bits[i];
        }
      }

      bool v1 = (v & BitMask{1}) != 0;
      u_bits[1] = v1 ? 1u : 0u;

      bool v2 = ((v >> 1) & BitMask{1}) != 0;
      u_bits[2] = static_cast<unsigned>(v2 ? 1u : 0u) ^ u_bits[1] ^ (parity & 1u);

      BitMask u = 0;
      for (std::size_t i = 1; i <= dim - 1; i++) {
        if (u_bits[i] & 1u) {
          u |= BitMask{1} << (i - 1);
        }
      }
      return u;
    }

    BitMask neighbour_dim_rec(BitMask x, std::size_t dim_idx, std::size_t dim) const {
      if (dim == 0) {
        throw std::logic_error("TwistedCubeConnected: dim == 0");
      }

      if (dim <= 2) {
        if (dim_idx >= dim) {
          throw std::out_of_range("TwistedCubeConnected: dim_idx out of range (base)");
        }
        BitMask mask = BitMask{1} << dim_idx;
        return x ^ mask;
      }

      if (dim_idx < dim - 1) {
        BitMask msb = x & (BitMask{1} << (dim - 1));
        BitMask suffix = x & ~ (BitMask{1} << (dim - 1));

        BitMask suffix_nb = neighbour_dim_rec(suffix, dim_idx, dim - 1);
        return msb | suffix_nb;
      }

      if (dim_idx == dim - 1) {
        bool top = ((x >> (dim - 1)) & BitMask{1}) != 0;
        if (!top) {
          return cross_from_zero(x, dim);
        } else {
          return cross_from_one(x, dim);
        }
      }

      throw std::out_of_range("TwistedCubeConnected: dim_idx >= dim");
    }

  public:
    using node_type = BitMask;

    explicit TwistedCubeConnected(std::size_t dim) : n(dim), num_nodes(static_cast<std::size_t>(pow2(dim))) {
      if (n == 0) {
        throw std::invalid_argument("TwistedCubeConnected: dimension must be > 0");
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
      for (std::size_t dim_idx = 0; dim_idx < n; dim_idx++) {
        BitMask nb = neighbour_at_impl(x, dim_idx);
        f(nb);
      }
    }

    std::size_t degree_impl(const BitMask&) const {
      return n;
    }

    BitMask neighbour_at_impl(const BitMask& x, std::size_t i) const {
      if (i >= n) {
        throw std::out_of_range("TwistedCubeConnected: neighbour index out of range");
      }
      return neighbour_dim_rec(x, i, n);
    }

    std::size_t dim_count() const {
      return n;
    }

    bool dim_aligned(BitMask a, BitMask b, std::size_t dim_idx) const {
      if (dim_idx >= n) {
        throw std::out_of_range("TwistedCubeConnected: dim_aligned dim out of range");
      }
      BitMask mask = BitMask{1} << dim_idx;
      return ((a ^ b) & mask) == 0;
    }

    BitMask move_to(BitMask from, BitMask to, std::size_t dim_idx) const {
      if (dim_idx >= n) {
        throw std::out_of_range("TwistedCubeConnected: move_to dim out of range");
      }
      BitMask mask = BitMask{1} << dim_idx;
      if (((from ^ to) & mask) != 0) {
        return neighbour_at_impl(from, dim_idx);
      }
      return from;
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
