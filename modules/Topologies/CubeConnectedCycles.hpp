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
      for (std::size_t c = 0; c < num_cube; ++c) {
        for (std::size_t i = 0; i < n; ++i) {
          CCCNode x{
            static_cast<BitMask>(c),
            static_cast<std::uint32_t>(i)
          };
          f(x);
        }
      }
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

    // Deterministic ordering:
    //  i = 0 -> cycle +1
    //  i = 1 -> cycle -1
    //  i = 2 -> cube edge
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

      // i == 2: cube edge
      {
        BitMask mask = BitMask{1} << x.pos;
        y.coord = x.coord ^ mask;
        return y;
      }
    }

    // TODO FIX ALL INTERFACES AND REMAKE DOR
    std::size_t dim_count() const {
      return 0; // no meaningful dimension-order routing here
    }

    bool dim_aligned(const CCCNode&, const CCCNode&, std::size_t) const {
      throw std::logic_error("CubeConnectedCycles: dim_aligned not supported");
    }

    CCCNode move_to(const CCCNode& from, const CCCNode&, std::size_t) const {
      // If someone insists on calling this, just return from and let them suffer.
      return from;
    }
  };

}
