#pragma once

#include <cstdint>
#include <cuda_runtime.h>

namespace route::cuda_detail {

  // csr graph for gpu mem
  struct CSRDevice {
    const uint32_t* row_offsets;]
    const uint32_t* col_indices;
    const uint8_t* node_alive;
    const uint8_t* edge_alive;
    const uint64_t* index_to_node;
    uint32_t num_nodes;
  };

  // returns CSR index of next hop or UINT32_MAX when no more progress can be made
  __device__ __forceinline__
  uint32_t dor_next_hop(const CSRDevice& csr, uint32_t cur_idx, uint32_t dest_idx) {
    uint64_t cur_node  = csr.index_to_node[cur_idx];
    uint64_t dest_node = csr.index_to_node[dest_idx];
    uint64_t diff = cur_node ^ dest_node;

    if (diff == 0) return cur_idx;

    uint64_t remaining = diff;
    while (remaining != 0) {
      int bit = __ffsll(static_cast<long long>(remaining)) - 1; // index of lsb
      uint64_t candidate_node = cur_node ^ (1ULL << bit);

      uint32_t start = csr.row_offsets[cur_idx];
      uint32_t end = csr.row_offsets[cur_idx + 1];

      for (uint32_t e_i = start; e_i < end; e_i++) {
        if (!csr.edge_alive[e_i]) continue;
        
        uint32_t v_idx = csr.col_indices[e_i];

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
