#pragma once

#include <vector>
#include <cstdint>
#include <type_traits>

#include "Helpers/Events.hpp"
#include "Topologies/CSR.hpp"

namespace engines {

  namespace cuda_detail {
    void run_flows_on_gpu(
        const uint32_t* h_row_offsets, uint32_t num_nodes,
        const uint32_t* h_col_indices, uint32_t num_edges,
        const uint8_t* h_node_alive,
        const uint8_t* h_edge_alive,
        const uint64_t* h_index_to_node,
        const uint32_t* h_flow_srcs,
        const uint32_t* h_flow_dests,
        uint32_t num_flows,
        uint32_t max_hops,
        uint32_t* h_out_hop_counts,
        uint8_t* h_out_failed,
        uint32_t* h_out_node_transits);
  }

  template <typename Topo>
  class CudaEngine {
  public:
    using Node  = typename Topo::node_type;
    using Event = helper::BasicEvents<Node>;

    static_assert(std::is_same_v<Node, uint64_t>,
                  "CudaEngine currently supports uint64_t node types only (Hypercube BitMask).");

    const std::vector<Event>& finished_flows() const {
      return finished_;
    }

    const std::vector<uint32_t>& node_transit_counts() const {
      return node_transits_;
    }

    template <typename TrafficTopo, typename TrafficGen>
    void runSim(const topo::CSRHost<Topo>& csr,
                const TrafficTopo& traffic_topo,
                TrafficGen&& traffic_gen) {
      auto flows = traffic_gen(traffic_topo);
      uint32_t num_flows = static_cast<uint32_t>(flows.size());
      uint32_t max_hops = static_cast<uint32_t>(csr.nodes);

      std::vector<uint32_t> flow_srcs(num_flows);
      std::vector<uint32_t> flow_dests(num_flows);
      for (uint32_t i = 0; i < num_flows; i++) {
        flow_srcs[i] = static_cast<uint32_t>(csr.node_to_index.at(flows[i].src));
        flow_dests[i] = static_cast<uint32_t>(csr.node_to_index.at(flows[i].dest));
      }

      std::vector<uint32_t> hop_counts(num_flows);
      std::vector<uint8_t> failed(num_flows);
      node_transits_.assign(csr.nodes, 0);

      cuda_detail::run_flows_on_gpu(
          csr.row_offsets.data(),
          static_cast<uint32_t>(csr.nodes),
          csr.col_indices.data(),
          static_cast<uint32_t>(csr.edges),
          csr.node_alive.data(),
          csr.edge_alive.data(),
          csr.index_to_node.data(),
          flow_srcs.data(),
          flow_dests.data(),
          num_flows,
          max_hops,
          hop_counts.data(),
          failed.data(),
          node_transits_.data());

      finished_.resize(num_flows);
      for (uint32_t i = 0; i < num_flows; i++) {
        Event& ev = finished_[i];
        ev.src = flows[i].src;
        ev.dest = flows[i].dest;
        ev.timestamp = hop_counts[i];
        ev.failed = (failed[i] != 0);
        ev.path.resize(hop_counts[i] + 1);
      }
    }

  private:
    std::vector<Event> finished_;
    std::vector<uint32_t> node_transits_;
  };

}
