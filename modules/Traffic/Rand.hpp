#pragma once

#include <random>
#include <vector>

#include "Helpers/Events.hpp"

namespace traffic {

  template <typename Topo>
  using NodeOf = typename Topo::node_type;

  template <typename Topo>
  using Flow = helper::BasicEvents<NodeOf<Topo>>;
  
  template <typename Topo>
  std::vector<Flow<Topo>> gen_rand_traffic(const Topo& topo) {
    using Node = NodeOf<Topo>;

    std::random_device rd;
    std::mt19937 rng{rd()};

    std::vector<Node> nodes;
    nodes.reserve(topo.node_count());
    topo.for_each_node([&](const Node& x) {
       nodes.push_back(x);
     });
    
    std::vector<Flow<Topo>> flows;
    flows.reserve(nodes.size());

    std::uniform_int_distribution<std::size_t> dist(0, nodes.size() - 1);

    for(const Node& src : nodes) {
      Node dest;

      dest = nodes[dist(rng)];

      flows.push_back(Flow<Topo>{src, dest});
    }
    return flows;
  }

}
