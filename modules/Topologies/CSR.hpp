#pragma once

#include <vector>
#include <unordered_map>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace Topo {
  template <typename Node>
  struct NodeIndex {
    std::vector<Node> index_to_node;
    std::unordered_map<Node, std::size_t> node_to_index;

    std::size_t size() const {return index_to_node.size();}

    std::size_t index_of(const Node& u) const {
      auto it = node_to_index.find(u);
      if(it == node_to_index.end()) throw std::logic_error("Node index not found.");
      return it->second;
    }
  };

  template <typename Topo>
  NodeIndex <typename Topo::node_type> build_node_index(const Topo& topo) {
    using Node = typename Topo::node_type;
    NodeIndex<Node> index;

    index.index_to_node.reserve(topo.node_count());

    topo.for_each_node_impl([&](const Node& u) {
                              std::size_t id = index.index_to_node.size();
                              index.index_to_node.push_back(u);
                              index.node_to_index.emplace(u, id);
                            });
    return index;
  }

  template <typename Topo>
  struct CSRHost {
    using Node = typename Topo::node_type;

    std::size_t nodes = 0;
    std::size_t edges = 0;

    std::vector<std::uint32_t> row_offsets;
    std::vector<std::uint32_t> col_indices;

    std::vector<std::uint8_t> node_alive;
    std::vector<std::uint8_t> edge_alive;
  };

  template <typename Topo>
  CSRHost<Topo> build_csr(const Topo& topo) {
    using Node = typename Topo::node_type;

    CSRHost<Topo> graph;

    // index stuff
    NodeIndex<Node> index = build_node_index(topo);
    graph.nodes = index.size();
    graph.index_to_node = index.index_to_node;

    graph.row_offsets.resize(graph.nodes + 1);

    // compute row offsets
    std::size_t edge_count = 0;
    for(std::size_t node_idx = 0; node_idx < graph.nodes; node_idx++) {
      const Node& u = graph.index_to_node[node_idx];
      std::size_t deg = topo.degree_impl(u);
      graph.row_offsets[node_idx] = static_cast<std::uint32_t> (edge_count);
      edge_count += deg;
    }

    graph.row_offsets[graph.nodes] = static_cast<std::uint32_t> (edge_count);

    graph.edges = edge_count;
    graph.col_indices.resize(graph.edges);

    // fault state settings - to modify later
    graph.node_alive.assign(graph.nodes, 1);
    graph.edge_alive.assign(graph.edges, 1);

    // second pass to actually fill adjacency
    for(std::size_t node_idx = 0; node_idx < graph.nodes; node_idx++) {
      const Node& u = graph.index_to_node[node_idx];
      std::size_t deg = topo.degree_impl(u);
      std::uint32_t start = graph.row_offsets[node_idx];

      for(std::size_t i = 0; i < deg; i++) {
        Node v = topo.neighbour_at_impl(u, i);

        std::size_t v_idx = index.index_of(v);
        graph.col_indices[start + i] = static_cast<std::uint32_t> (v_idx);
      }
    }

    return graph;
  }
}
