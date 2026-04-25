#pragma once

#include <cstdint>
#include <cuda_runtime.h>
#include "Routers/CudaDOR.cuh"

namespace engines::cuda_detail {

  __global__
  void simulate_flows_kernel(
      route::cuda_detail::CSRDevice csr,
      const uint32_t* __restrict__ flow_srcs,
      const uint32_t* __restrict__ flow_dests,
      uint32_t  num_flows,
      uint32_t  max_hops,
      uint32_t* __restrict__ out_hop_counts,
      uint8_t*  __restrict__ out_failed,
      uint32_t* __restrict__ node_transit_counts)
  {
    uint32_t tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid >= num_flows) return;

    uint32_t cur  = flow_srcs[tid];
    uint32_t dest = flow_dests[tid];
    uint32_t hops = 0;

    while (cur != dest && hops < max_hops) {
      uint32_t next = route::cuda_detail::dor_next_hop(csr, cur, dest);
      if (next == UINT32_MAX) {
        out_hop_counts[tid] = hops;
        out_failed[tid] = 1;
        return;
      }

      // Count intermediate transits (not src, not dest)
      if (hops > 0) {
        atomicAdd(&node_transit_counts[cur], 1);
      }

      cur = next;
      hops++;
    }

    out_hop_counts[tid] = hops;
    out_failed[tid] = (cur != dest) ? 1 : 0;
  }

}
