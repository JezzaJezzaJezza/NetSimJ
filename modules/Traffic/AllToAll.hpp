#pragma once

#include <vector>

#include "Helpers/Events.hpp"

namespace traffic {

  template <typename Topo>
  std::vector<helper::BasicEvents<typename Topo::node_type>> gen_all_to_all_traffic(const Topo& topo) {
    using Node = typename Topo::node_type;
    using Flow = helper::BasicEvents<Node>;

    std::vector<Node> nodes;
    nodes.reserve(topo.node_count());
    topo.for_each_endpoint([&](const Node& x) {
      nodes.push_back(x);
    });

    std::vector<Flow> flows;
    flows.reserve(nodes.size() * (nodes.size() - 1));

    for (const Node& src : nodes) {
      for (const Node& dest : nodes) {
        if (src != dest) {
          flows.push_back(Flow{src, dest});
        }
      }
    }
    return flows;
  }

}
