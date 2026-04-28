#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include "Base.hpp"

namespace topo {

  using BitMask = std::uint64_t;

  // coord is the global coord of the cycle
  // pos is the actual node in the cycle
  struct CCCNode {
    BitMask coord;
    std::uint32_t pos;
  };

  inline bool operator==(const CCCNode& a, const CCCNode& b) {
    return a.coord == b.coord && a.pos == b.pos;
  }

  inline bool operator!=(const CCCNode& a, const CCCNode& b) {
    return !(a == b);
  }

  class CubeConnectedCycles
    : public BaseTopo<CubeConnectedCycles, CCCNode> {
  private:
    const std::size_t n;
    const std::size_t num_cube;
    const std::size_t num_nodes;

    static std::size_t pow2(std::size_t e) {
      if (e >= sizeof(BitMask) * 8) {
        throw std::runtime_error("CubeConnectedCycles: dim too large for BitMask");
      }
      return std::size_t{1} << e;
    }

  public:
    using node_type = CCCNode;

    explicit CubeConnectedCycles(std::size_t dim)
      : n(dim),
        num_cube(pow2(dim)),
        num_nodes(num_cube * dim) {

      if (n < 2) {
        throw std::invalid_argument("CubeConnectedCycles: dimension must be >= 2");
      }
    }

    std::size_t node_count_impl() const {
      return num_nodes;
    }

    template <typename F>
    void for_each_node_impl(F&& f) const {
      for (std::size_t c = 0; c < num_cube; c++) {
        for (std::size_t i = 0; i < n; i++) {
          CCCNode x{
            static_cast<BitMask>(c),
            static_cast<std::uint32_t>(i)
          };
          f(x);
        }
      }
    }

    template <typename F>
    void for_each_endpoint_impl(F&& f) const {
      for_each_node_impl(std::forward<F>(f));
    }

    template <typename F>
    void for_each_neighbour_impl(const CCCNode& x, F&& f) const {
      CCCNode y;

      y = x;
      y.pos = static_cast<std::uint32_t>((x.pos + 1) % n);
      f(y);

      y = x;
      y.pos = static_cast<std::uint32_t>((x.pos + n - 1) % n);
      f(y);

      y = x;
      BitMask mask = BitMask{1} << x.pos;
      y.coord = x.coord ^ mask;
      f(y);
    }

    std::size_t degree_impl(const CCCNode&) const {
      return 3;
    }

    CCCNode neighbour_at_impl(const CCCNode& x, std::size_t i) const {
      if (i >= 3) {
        throw std::out_of_range("CubeConnectedCycles: neighbour index out of range");
      }

      CCCNode y = x;

      if (i == 0) {
        y.pos = static_cast<std::uint32_t>((x.pos + 1) % n);
        return y;
      }
      if (i == 1) {
        y.pos = static_cast<std::uint32_t>((x.pos + n - 1) % n);
        return y;
      }

      BitMask mask = BitMask{1} << x.pos;
      y.coord = x.coord ^ mask;
      return y;
    }

    std::string node_to_string_impl(const CCCNode& x) const {
      std::string s;
      s.reserve(n + 8);

      for (std::size_t i = 0; i < n; i++) {
        std::size_t bit = n - 1 - i;
        bool one = (x.coord >> bit) & 1ULL;
        s.push_back(one ? '1' : '0');
      }

      s.push_back('@');
      s += std::to_string(x.pos);
      return s;
    }

    std::size_t dim_count_impl() const {
      return n;
    }

  };
}


namespace std {
  template<>
  struct hash<topo::CCCNode> {
    std::size_t operator()(const topo::CCCNode& x) const noexcept {
      std::size_t h1 = std::hash<topo::BitMask>{}(x.coord);
      std::size_t h2 = std::hash<std::uint32_t>{}(x.pos);
      return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
    }
  };
}
