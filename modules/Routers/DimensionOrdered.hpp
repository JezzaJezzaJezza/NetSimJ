#include <type_traits>

namespace route {
  template <typename Topo> typename Topo::node_type
  DOR_next_hop(const Topo& topo, const typename Topo::node_type& cur, const typename Topo::node_type& dest) {
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

      // Try dimensions in order; respect faults via for_each_neighbour
      for (std::size_t dim = 0; dim < dim_count; ++dim) {
        Unsigned mask = Unsigned{1} << dim;
        if (!(diff & mask)) continue;  // already aligned in this dim

        Node candidate = static_cast<Node>(u ^ mask);

        bool live = false;
        topo.for_each_neighbour(cur, [&](Node nb) {
          if (nb == candidate) {
            live = true;   // neighbour exists and is non-faulty
          }
        });

        if (live) {
          return candidate;
        }
      }

      // No differing-bit neighbour is live.
      // Fallback: pick any live neighbour if one exists, else stay put.
      Node fallback = cur;
      bool has_fallback = false;

      topo.for_each_neighbour(cur, [&](Node nb) {
        if (!has_fallback) {
          fallback = nb;
          has_fallback = true;
        }
      });

      return has_fallback ? fallback : cur;
    }
}
