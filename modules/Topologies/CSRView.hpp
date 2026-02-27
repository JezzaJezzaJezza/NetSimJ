#pragma once

#include "CSR.hpp"
#include "Base.hpp"

namespace topo {
  template <typename Topo>
  class CSRView : public BaseTopo<CSRView<Topo>, std::size_t> {
    public:
      using node_type = std::size_t;

      explicit CSRView(const CSRHost<Topo>& graph, const Topo& base) : graph_(graph), base_(base) {}

      std::size_t node_count_impl() const {
        return graph_.nodes;
      }

      template <typename F>
      void for_each_node_impl(F&& f) const {
        for (std::size_t i = 0; i < graph_.nodes; i++) {
          if (!graph_.node_alive[i]) continue;
          f(i);
        }
      }

      template <typename F>
      void for_each_endpoint_impl(F&& f) const {
        for (std::size_t i = 0; i < graph_.nodes; ++i) {
          if (!graph_.node_alive[i]) continue;
          if (!graph_.is_endpoint[i]) continue;
          f(i);
        }
      }

      template <typename F>
      void for_each_neighbour_impl(const std::size_t& u_idx, F&& f) const {
        if (!graph_.node_alive[u_idx]) return;
        auto start = graph_.row_offsets[u_idx];
        auto end = graph_.row_offsets[u_idx + 1];

        for (std::uint32_t i = start; i < end; i++) {
          if (!graph_.edge_alive[i]) continue;

          std::size_t v_idx = graph_.col_indices[i];

          if (!graph_.node_alive[v_idx]) continue;
          f(v_idx);
        }
      }

      std::size_t degree_impl(const std::size_t& u_idx) const {
        if (!graph_.node_alive[u_idx]) return 0;
        auto start = graph_.row_offsets[u_idx];
        auto end = graph_.row_offsets[u_idx + 1];
        // TODO subtract faulty edges from returned degree
        return end - start;
      }

      std::size_t neighbour_at_impl(const std::size_t& u_idx, std::size_t i) const {
        auto start = graph_.row_offsets[u_idx];
        auto end = graph_.row_offsets[u_idx + 1];

        if (start + i >= end) {
          throw std::out_of_range("CSRTopoView: neighbour index out of range.");
        }

        return graph_.col_indices[start + i];
      }

      std::string node_to_string_impl(const std::size_t& idx) const {
        const auto& node = graph_.index_to_node[idx];
        return base_.node_to_string(node);
      }
    
    private:
      const CSRHost<Topo>& graph_;
      const Topo& base_;
  };
}
