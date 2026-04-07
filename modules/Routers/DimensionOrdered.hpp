#include <optional>
#include <type_traits>

namespace route {

  template <typename Topo>
  std::optional<typename Topo::node_type>
  hypercube_DOR(const Topo& topo,
                const typename Topo::node_type& cur,
                const typename Topo::node_type& dest) {
    using Node = typename Topo::node_type;
    using Unsigned = std::make_unsigned_t<Node>;

    if (cur == dest) return cur;

    // Infer dimension from node_count, assuming 2^n nodes
    std::size_t dim_count = 0;
    {
      std::size_t N = topo.node_count();
      while ((std::size_t{1} << dim_count) < N) {
        dim_count++;
      }
    }

    Unsigned u = static_cast<Unsigned>(cur);
    Unsigned v = static_cast<Unsigned>(dest);
    Unsigned diff = u ^ v;  // bits where they differ

    // Helper: check if 'candidate' is an actual neighbour of 'cur'
    auto is_neighbour = [&](const Node& candidate) -> bool {
      bool found = false;
      topo.for_each_neighbour(cur, [&](const Node& nb) {
        if (!found && nb == candidate) {
          found = true;
        }
      });
      return found;
    };

    // Try dimensions in order; respect faults via for_each_neighbour
    for (std::size_t dim = 0; dim < dim_count; ++dim) {
      Unsigned mask = Unsigned{1} << dim;
      if (!(diff & mask)) continue;  // already aligned in this dim

      Node candidate = static_cast<Node>(u ^ mask);

      if (is_neighbour(candidate)) {
        return candidate;
      }
    }

    // No differing-bit neighbour is live — route is blocked
    return std::nullopt;
  }

}
