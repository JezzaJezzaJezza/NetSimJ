#pragma once

#include "CSR.hpp"
#include "Base.hpp"

namespace topo {

  template <typename Topo>
  class CSRView : public BaseTopo<CSRView<Topo>, typename Topo::node_type> {
  public:
    using Node = typename Topo::node_type;
    using node_type = Node;

    CSRView(const CSRHost<Topo>& graph, const Topo& base)
      : graph_(graph), base_(base) {}

    std::size_t node_count_impl() const {
      return graph_.nodes;
    }

    std::size_t dim_count_impl() const {
      return base_.dim_count();
    }

    template <typename F>
    void for_each_node_impl(F&& f) const {
      for (std::size_t idx = 0; idx < graph_.nodes; ++idx) {
        if (!graph_.node_alive[idx]) continue;
        const Node& u = graph_.index_to_node[idx];
        f(u);
      }
    }

    template <typename F>
    void for_each_endpoint_impl(F&& f) const {
      for (std::size_t idx = 0; idx < graph_.nodes; ++idx) {
        if (!graph_.node_alive[idx]) continue;
        if (!graph_.is_endpoint[idx]) continue;
        const Node& u = graph_.index_to_node[idx];
        f(u);
      }
    }

    // Neighbours: map Node -> idx, walk adjacency, map back idx -> Node
    template <typename F>
    void for_each_neighbour_impl(const Node& u, F&& f) const {
      auto it = graph_.node_to_index.find(u);
      if (it == graph_.node_to_index.end()) {
        throw std::logic_error("CSRView: node not in CSR graph");
      }
      std::size_t u_idx = it->second;
      if (!graph_.node_alive[u_idx]) return;

      auto start = graph_.row_offsets[u_idx];
      auto end = graph_.row_offsets[u_idx + 1];

      for (std::uint32_t ei = start; ei < end; ++ei) {
        if (!graph_.edge_alive[ei]) continue;

        std::size_t v_idx = graph_.col_indices[ei];
        if (!graph_.node_alive[v_idx]) continue;

        const Node& v = graph_.index_to_node[v_idx];
        f(v);
      }
    }

    std::size_t degree_impl(const Node& u) const {
      auto it = graph_.node_to_index.find(u);
      if (it == graph_.node_to_index.end()) {
        throw std::logic_error("CSRView: node not in CSR graph");
      }
      std::size_t u_idx = it->second;
      if (!graph_.node_alive[u_idx]) return 0;

      auto start = graph_.row_offsets[u_idx];
      auto end = graph_.row_offsets[u_idx + 1];

      return end - start;
    }

    Node neighbour_at_impl(const Node& u, std::size_t i) const {
      auto it = graph_.node_to_index.find(u);
      if (it == graph_.node_to_index.end()) {
        throw std::logic_error("CSRView: node not in CSR graph");
      }
      std::size_t u_idx = it->second;
      if (!graph_.node_alive[u_idx]) {
        throw std::out_of_range("CSRView: dead node has no neighbours");
      }

      auto start = graph_.row_offsets[u_idx];
      auto end = graph_.row_offsets[u_idx + 1];

      std::size_t logical = 0;
      for (std::uint32_t ei = start; ei < end; ei++) {
        if (!graph_.edge_alive[ei]) continue;
        std::size_t v_idx = graph_.col_indices[ei];
        if (!graph_.node_alive[v_idx]) continue;

        if (logical == i) {
          return graph_.index_to_node[v_idx];
        }
        logical++;
      }

      throw std::out_of_range("CSRView: neighbour index out of range");
    }

    // printing: just forward to the base topo
    std::string node_to_string_impl(const Node& x) const {
      return base_.node_to_string(x);
    }

    // optional accessors if you *really* need them later
    const CSRHost<Topo>& graph() const {return graph_;}
    const Topo& base() const {return base_;}

  private:
    const CSRHost<Topo>& graph_;
    const Topo& base_;
  };
}
