#include "CudaEngine.cuh"
#include <cstdio>

#define CUDA_CHECK(call)                                                      \
  do {                                                                        \
    cudaError_t err = (call);                                                 \
    if (err != cudaSuccess) {                                                 \
      std::fprintf(stderr, "CUDA error at %s:%d — %s\n",                     \
                   __FILE__, __LINE__, cudaGetErrorString(err));              \
      std::exit(1);                                                          \
    }                                                                        \
  } while (0)

namespace engines::cuda_detail {

  void run_flows_on_gpu(
      const uint64_t* h_srcs,
      const uint64_t* h_dests,
      uint32_t num_flows,
      uint32_t dim,
      bool record_paths,
      uint32_t* h_out_hop_counts,
      uint8_t*  h_out_failed,
      uint64_t* h_out_paths,
      uint32_t  max_path_len)
  {
    uint64_t* d_srcs        = nullptr;
    uint64_t* d_dests       = nullptr;
    uint32_t* d_hop_counts  = nullptr;
    uint8_t*  d_failed      = nullptr;
    uint64_t* d_paths       = nullptr;

    std::size_t flow_bytes  = static_cast<std::size_t>(num_flows);

    CUDA_CHECK(cudaMalloc(&d_srcs,       flow_bytes * sizeof(uint64_t)));
    CUDA_CHECK(cudaMalloc(&d_dests,      flow_bytes * sizeof(uint64_t)));
    CUDA_CHECK(cudaMalloc(&d_hop_counts, flow_bytes * sizeof(uint32_t)));
    CUDA_CHECK(cudaMalloc(&d_failed,     flow_bytes * sizeof(uint8_t)));

    if (record_paths) {
      std::size_t path_bytes = flow_bytes * max_path_len * sizeof(uint64_t);
      CUDA_CHECK(cudaMalloc(&d_paths, path_bytes));
    }

    CUDA_CHECK(cudaMemcpy(d_srcs,  h_srcs,  flow_bytes * sizeof(uint64_t), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_dests, h_dests, flow_bytes * sizeof(uint64_t), cudaMemcpyHostToDevice));

    constexpr uint32_t block_size = 256;
    uint32_t grid_size = (num_flows + block_size - 1) / block_size;

    simulate_flows_kernel<<<grid_size, block_size>>>(
        d_srcs, d_dests, num_flows, dim,
        d_hop_counts, d_failed, d_paths, max_path_len);

    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());

    CUDA_CHECK(cudaMemcpy(h_out_hop_counts, d_hop_counts, flow_bytes * sizeof(uint32_t), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_out_failed,     d_failed,     flow_bytes * sizeof(uint8_t),  cudaMemcpyDeviceToHost));

    if (record_paths && h_out_paths) {
      std::size_t path_bytes = flow_bytes * max_path_len * sizeof(uint64_t);
      CUDA_CHECK(cudaMemcpy(h_out_paths, d_paths, path_bytes, cudaMemcpyDeviceToHost));
    }

    cudaFree(d_srcs);
    cudaFree(d_dests);
    cudaFree(d_hop_counts);
    cudaFree(d_failed);
    cudaFree(d_paths);
  }

}
