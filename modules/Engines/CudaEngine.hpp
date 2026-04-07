#pragma once

#include <vector>
#include <cstdint>
#include <type_traits>

#include "Helpers/Events.hpp"

namespace engines {

  // Implemented in CudaEngine.cu (compiled by nvcc)
  namespace cuda_detail {
    void run_flows_on_gpu(
        const uint64_t* srcs,
        const uint64_t* dests,
        uint32_t num_flows,
        uint32_t dim,
        bool record_paths,
        uint32_t* out_hop_counts,
        uint8_t*  out_failed,
        uint64_t* out_paths,
        uint32_t  max_path_len);
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

    template <typename TrafficGen, typename Router>
    void runSim(const Topo& topo, TrafficGen&& traffic_gen, Router&& /*router*/) {
      auto flows = traffic_gen(topo);
      uint32_t num_flows = static_cast<uint32_t>(flows.size());
      uint32_t dim = static_cast<uint32_t>(topo.dim());
      uint32_t max_path_len = dim + 1;

      // Extract src/dest into flat arrays
      std::vector<uint64_t> srcs(num_flows);
      std::vector<uint64_t> dests(num_flows);
      for (uint32_t i = 0; i < num_flows; ++i) {
        srcs[i]  = flows[i].src;
        dests[i] = flows[i].dest;
      }

      // Allocate host output buffers
      std::vector<uint32_t> hop_counts(num_flows);
      std::vector<uint8_t>  failed(num_flows);
      std::vector<uint64_t> paths(static_cast<std::size_t>(num_flows) * max_path_len);

      cuda_detail::run_flows_on_gpu(
          srcs.data(), dests.data(), num_flows, dim,
          /*record_paths=*/true,
          hop_counts.data(), failed.data(),
          paths.data(), max_path_len);

      // Convert results back to Event format
      finished_.resize(num_flows);
      for (uint32_t i = 0; i < num_flows; ++i) {
        Event& ev = finished_[i];
        uint32_t hops = hop_counts[i];

        ev.dest = static_cast<Node>(dests[i]);
        ev.src = static_cast<Node>(dests[i]);
        ev.timestamp = hops;
        ev.failed = (failed[i] != 0);

        ev.path.resize(hops + 1);
        const uint64_t* row = &paths[static_cast<std::size_t>(i) * max_path_len];
        for (uint32_t j = 0; j <= hops; ++j) {
          ev.path[j] = static_cast<Node>(row[j]);
        }
      }
    }

  private:
    std::vector<Event> finished_;
  };

}
