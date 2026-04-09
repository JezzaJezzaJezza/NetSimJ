#pragma once

#include <cstdint>
#include <cuda_runtime.h>

namespace route::cuda_detail {

  // Flat CSR graph for GPU device memory.
  struct CSRDevice {
    const uint32_t* row_offsets;   // [num_nodes + 1]
    const uint32_t* col_indices;   // [num_edges]
    const uint8_t*  node_alive;    // [num_nodes]
    const uint8_t*  edge_alive;    // [num_edges]
    const uint64_t* index_to_node; // [num_nodes] — BitMask node values
    uint32_t        num_nodes;
  };

  // Dimension-ordered routing over a CSR graph with fault flags.
  // Returns the CSR index of the next hop, or UINT32_MAX when blocked.
  __device__ __forceinline__
  uint32_t dor_next_hop(const CSRDevice& csr, uint32_t cur_idx, uint32_t dest_idx) {
    uint64_t cur_node  = csr.index_to_node[cur_idx];
    uint64_t dest_node = csr.index_to_node[dest_idx];
    uint64_t diff = cur_node ^ dest_node;

    if (diff == 0) return cur_idx;

    // Walk differing bits lowest-first (standard DOR order).
    uint64_t remaining = diff;
    while (remaining != 0) {
      int bit = __ffsll(static_cast<long long>(remaining)) - 1;
      uint64_t candidate_node = cur_node ^ (1ULL << bit);

      // Search live neighbours in CSR for the candidate.
      uint32_t start = csr.row_offsets[cur_idx];
      uint32_t end   = csr.row_offsets[cur_idx + 1];

      for (uint32_t ei = start; ei < end; ++ei) {
        if (!csr.edge_alive[ei]) continue;
        uint32_t v_idx = csr.col_indices[ei];
        if (!csr.node_alive[v_idx]) continue;
        if (csr.index_to_node[v_idx] == candidate_node) {
          return v_idx;
        }
      }

      remaining &= remaining - 1; // clear lowest set bit
    }

    return UINT32_MAX;
  }

}
