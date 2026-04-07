#pragma once

#include <cstdint>
#include <cuda_runtime.h>

namespace engines::cuda_detail {

  __device__ __forceinline__
  uint64_t dor_next_hop(uint64_t cur, uint64_t dest) {
    uint64_t diff = cur ^ dest;
    if (diff == 0) return cur;
    int bit = __ffsll(static_cast<long long>(diff)) - 1;
    return cur ^ (1ULL << bit);
  }

  __global__
  void simulate_flows_kernel(
      const uint64_t* __restrict__ srcs,
      const uint64_t* __restrict__ dests,
      uint32_t  num_flows,
      uint32_t  dim,
      uint32_t* __restrict__ out_hop_counts,
      uint8_t*  __restrict__ out_failed,
      uint64_t* __restrict__ out_paths,
      uint32_t  max_path_len)
  {
    uint32_t tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid >= num_flows) return;

    uint64_t cur  = srcs[tid];
    uint64_t dest = dests[tid];
    uint32_t hops = 0;

    if (out_paths) {
      out_paths[static_cast<uint64_t>(tid) * max_path_len] = cur;
    }

    while (cur != dest && hops < dim) {
      cur = dor_next_hop(cur, dest);
      hops++;

      if (out_paths && hops < max_path_len) {
        out_paths[static_cast<uint64_t>(tid) * max_path_len + hops] = cur;
      }
    }

    out_hop_counts[tid] = hops;
    out_failed[tid] = (cur != dest) ? 1 : 0;
  }

}
