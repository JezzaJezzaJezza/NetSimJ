#include <unordered_set>
#include <vector>
#include <cassert>

namespace helper {
  template <typename Topo>
  void check_basic_topology(const Topo& topo) {
    using Node = typename Topo::node_type;

    std::vector<Node> nodes;
    nodes.reserve(topo.node_count());

    topo.for_each_node([&](const Node& x) {
      nodes.push_back(x);
    });

    // 1. node_count() consistent
    assert(nodes.size() == topo.node_count());

    // 2. no duplicate nodes (if Node is hashable / comparable)
    std::unordered_set<Node> uniq;
    for (auto& x : nodes) {
      uniq.insert(x);
    }
    assert(uniq.size() == nodes.size());

    // 3. degree / neighbours / neighbour_at consistency
    for (auto& x : nodes) {
      std::vector<Node> neigh;
      topo.for_each_neighbour(x, [&](const Node& y) {
        neigh.push_back(y);
      });

      auto d = topo.degree(x);
      assert(d == neigh.size());

      for (std::size_t i = 0; i < d; ++i) {
        Node y = topo.neighbour_at(x, i);

        // y must appear in neigh
        bool found = false;
        for (auto& z : neigh) {
          if (z == y) { found = true; break; }
        }
        assert(found && "neighbour_at not consistent with for_each_neighbour");
      }

      // 4. symmetry: if undirected, x must be in neighbours(y)
      for (auto& y : neigh) {
        bool back = false;
        topo.for_each_neighbour(y, [&](const Node& z) {
          if (z == x) back = true;
        });
        assert(back && "edge not symmetric");
      }
    }
  }
}
