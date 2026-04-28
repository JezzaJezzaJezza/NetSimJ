#include <optional>

namespace route {

  template <typename Topo>
  std::optional<typename Topo::node_type>
  CCC_next_hop(const Topo& topo,
               const typename Topo::node_type& cur,
               const typename Topo::node_type& dest) {
    using Node = typename Topo::node_type;
    using BitMask = decltype(Node{}.coord);

    const std::size_t n = topo.dim_count();

    auto is_neighbour = [&](const Node& candidate) -> bool {
      bool found = false;
      topo.for_each_neighbour(cur, [&](const Node& nb) {
        if (!found && nb == candidate) {
          found = true;
        }
      });
      return found;
    };

    if (cur.coord == dest.coord) {
      if (cur.pos == dest.pos) {
        return cur;
      }

      std::size_t i = cur.pos;
      std::size_t j = dest.pos;

      std::size_t forward = (j + n - i) % n;
      std::size_t backward = (i + n - j) % n;
      bool go_forward = (forward <= backward);

      std::uint8_t new_pos = static_cast<std::uint8_t>(
        go_forward ? ((i + 1) % n) : ((i + n - 1) % n)
      );

      Node candidate{cur.coord, new_pos};
      if (is_neighbour(candidate)) {
        return candidate;
      } else {
        return std::nullopt;
      }
    }

    BitMask diff = cur.coord ^ dest.coord;
    std::size_t start = cur.pos;
    std::size_t target_bit = n;

    for (std::size_t offset = 0; offset < n; offset++) {
      std::size_t idx = (start + offset) % n;
      BitMask mask = BitMask{1} << idx;
      if (diff & mask) {
        target_bit = idx;
        break;
      }
    }

    if (target_bit == n) {
      std::uint8_t new_pos = static_cast<std::uint8_t>((cur.pos + 1) % n);
      Node candidate{cur.coord, new_pos};
      if (is_neighbour(candidate)) {
        return candidate;
      } else {
        return std::nullopt;
      }
    }

    if (cur.pos != target_bit) {
      std::uint8_t new_pos = static_cast<std::uint8_t>((cur.pos + 1) % n);
      Node candidate{cur.coord, new_pos};
      if (is_neighbour(candidate)) {
        return candidate;
      } else {
        return std::nullopt;
      }
    }

    BitMask mask = BitMask{1} << cur.pos;
    BitMask new_coord = cur.coord ^ mask;
    Node candidate{new_coord, cur.pos};
    if (is_neighbour(candidate)) {
      return candidate;
    } else {
      return std::nullopt;
    }
  }

}
