#pragma once

#include <vector>
#include <unordered_map>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <random>

namespace topo {
  template <typename Node>
  struct NodeIndex {
    std::vector<Node> index_to_node;
    std::unordered_map<Node, std::size_t> node_to_index;

    std::size_t size() const { return index_to_node.size(); }

    std::size_t index_of(const Node& u) const {
      auto it = node_to_index.find(u);
      if (it == node_to_index.end()) {
        throw std::logic_error("Node index not found.");
      }
      return it->second;
    }
  };

  template <typename Topo>
  NodeIndex<typename Topo::node_type> build_node_index(const Topo& topo) {
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

    std::vector<Node> index_to_node;
    std::unordered_map<Node, std::size_t> node_to_index;

    std::vector<std::uint8_t> is_endpoint;
  };

  template <typename Topo, typename URBG>
  CSRHost<Topo> build_csr(const Topo& topo,
                          double node_fault_prob,
                          double edge_fault_prob,
                          URBG& rng) {
    using Node = typename Topo::node_type;

    if (node_fault_prob < 0.0 || node_fault_prob > 1.0) {
      throw std::invalid_argument("node_fault_prob must be in [0,1]");
    }
    if (edge_fault_prob < 0.0 || edge_fault_prob > 1.0) {
      throw std::invalid_argument("edge_fault_prob must be in [0,1]");
    }

    CSRHost<Topo> graph;

    NodeIndex<Node> index = build_node_index(topo);
    graph.nodes = index.size();
    graph.index_to_node = index.index_to_node;
    graph.node_to_index = index.node_to_index;

    graph.row_offsets.resize(graph.nodes + 1);

    std::size_t edge_count = 0;
    for (std::size_t node_idx = 0; node_idx < graph.nodes; node_idx++) {
      const Node& u = index.index_to_node[node_idx];
      std::size_t deg = topo.degree_impl(u);
      graph.row_offsets[node_idx] = static_cast<std::uint32_t>(edge_count);
      edge_count += deg;
    }

    graph.row_offsets[graph.nodes] = static_cast<std::uint32_t>(edge_count);

    graph.edges = edge_count;
    graph.col_indices.resize(graph.edges);

    graph.node_alive.assign(graph.nodes, 1);
    graph.edge_alive.assign(graph.edges, 1);

    for (std::size_t node_idx = 0; node_idx < graph.nodes; node_idx++) {
      const Node& u = index.index_to_node[node_idx];
      std::size_t deg = topo.degree_impl(u);
      std::uint32_t start = graph.row_offsets[node_idx];

      for (std::size_t i = 0; i < deg; i++) {
        Node v = topo.neighbour_at_impl(u, i);
        std::size_t v_idx = index.index_of(v);
        graph.col_indices[start + i] = static_cast<std::uint32_t>(v_idx);
      }
    }

    graph.is_endpoint.assign(graph.nodes, 0);
    topo.for_each_endpoint_impl([&](const Node& u) {
      std::size_t idx_u = index.index_of(u);
      graph.is_endpoint[idx_u] = 1;
    });

    std::bernoulli_distribution node_fault(node_fault_prob);
    for (std::size_t i = 0; i < graph.nodes; i++) {
      if (node_fault(rng)) {
        graph.node_alive[i] = 0;
      }
    }

    std::bernoulli_distribution edge_fault(edge_fault_prob);
    for (std::size_t ei = 0; ei < graph.edges; ei++) {
      if (edge_fault(rng)) {
        graph.edge_alive[ei] = 0;
      }
    }

    return graph;
  }

  template <typename Topo>
  CSRHost<Topo> build_csr(const Topo& topo) {
    CSRHost<Topo> graph;

    NodeIndex<typename Topo::node_type> index = build_node_index(topo);
    graph.nodes = index.size();
    graph.index_to_node = index.index_to_node;
    graph.node_to_index = index.node_to_index;

    graph.row_offsets.resize(graph.nodes + 1);

    std::size_t edge_count = 0;
    for (std::size_t node_idx = 0; node_idx < graph.nodes; node_idx++) {
      const auto& u = index.index_to_node[node_idx];
      std::size_t deg = topo.degree_impl(u);
      graph.row_offsets[node_idx] = static_cast<std::uint32_t>(edge_count);
      edge_count += deg;
    }

    graph.row_offsets[graph.nodes] = static_cast<std::uint32_t>(edge_count);

    graph.edges = edge_count;
    graph.col_indices.resize(graph.edges);

    graph.node_alive.assign(graph.nodes, 1);
    graph.edge_alive.assign(graph.edges, 1);

    for (std::size_t node_idx = 0; node_idx < graph.nodes; node_idx++) {
      const auto& u = index.index_to_node[node_idx];
      std::size_t deg = topo.degree_impl(u);
      std::uint32_t start = graph.row_offsets[node_idx];

      for (std::size_t i = 0; i < deg; i++) {
        auto v = topo.neighbour_at_impl(u, i);
        std::size_t v_idx = index.index_of(v);
        graph.col_indices[start + i] = static_cast<std::uint32_t>(v_idx);
      }
    }

    graph.is_endpoint.assign(graph.nodes, 0);
    topo.for_each_endpoint_impl([&](const auto& u) {
      std::size_t idx_u = index.index_of(u);
      graph.is_endpoint[idx_u] = 1;
    });

    return graph;
  }
}
