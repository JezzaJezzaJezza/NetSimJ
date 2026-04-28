#include "CudaEngine.cuh"
#include <cstdio>

namespace engines::cuda_detail {

  void run_flows_on_gpu(const uint32_t* h_row_offsets, uint32_t num_nodes, const uint32_t* h_col_indices, uint32_t num_edges,
      const uint8_t* h_node_alive, const uint8_t* h_edge_alive, const uint64_t* h_index_to_node, const uint32_t* h_flow_srcs,
      const uint32_t* h_flow_dests, uint32_t num_flows, uint32_t max_hops, uint32_t* h_out_hop_counts, uint8_t* h_out_failed,
      uint32_t* h_out_node_transits) {

    uint32_t* d_row_offsets = nullptr;
    uint32_t* d_col_indices = nullptr;
    uint8_t* d_node_alive = nullptr;
    uint8_t* d_edge_alive = nullptr;
    uint64_t* d_index_to_node = nullptr;

    cudaMalloc(&d_row_offsets, (num_nodes + 1) * sizeof(uint32_t));
    cudaMalloc(&d_col_indices, num_edges * sizeof(uint32_t));
    cudaMalloc(&d_node_alive, num_nodes * sizeof(uint8_t));
    cudaMalloc(&d_edge_alive, num_edges * sizeof(uint8_t));
    cudaMalloc(&d_index_to_node, num_nodes * sizeof(uint64_t));

    cudaMemcpy(d_row_offsets, h_row_offsets, (num_nodes + 1) * sizeof(uint32_t), cudaMemcpyHostToDevice);
    cudaMemcpy(d_col_indices, h_col_indices, num_edges * sizeof(uint32_t), cudaMemcpyHostToDevice);
    cudaMemcpy(d_node_alive, h_node_alive, num_nodes * sizeof(uint8_t), cudaMemcpyHostToDevice);
    cudaMemcpy(d_edge_alive, h_edge_alive, num_edges * sizeof(uint8_t), cudaMemcpyHostToDevice);
    cudaMemcpy(d_index_to_node, h_index_to_node, num_nodes * sizeof(uint64_t), cudaMemcpyHostToDevice);

    uint32_t* d_flow_srcs = nullptr;
    uint32_t* d_flow_dests = nullptr;
    uint32_t* d_hop_counts = nullptr;
    uint8_t* d_failed = nullptr;
    uint32_t* d_node_transits = nullptr;

    cudaMalloc(&d_flow_srcs, num_flows * sizeof(uint32_t));
    cudaMalloc(&d_flow_dests, num_flows * sizeof(uint32_t));
    cudaMalloc(&d_hop_counts, num_flows * sizeof(uint32_t));
    cudaMalloc(&d_failed, num_flows * sizeof(uint8_t));
    cudaMalloc(&d_node_transits, num_nodes * sizeof(uint32_t));

    cudaMemcpy(d_flow_srcs, h_flow_srcs, num_flows * sizeof(uint32_t), cudaMemcpyHostToDevice);
    cudaMemcpy(d_flow_dests, h_flow_dests, num_flows * sizeof(uint32_t), cudaMemcpyHostToDevice);
    cudaMemset(d_node_transits, 0, num_nodes * sizeof(uint32_t));

    route::cuda_detail::CSRDevice csr;
    csr.row_offsets = d_row_offsets;
    csr.col_indices = d_col_indices;
    csr.node_alive = d_node_alive;
    csr.edge_alive = d_edge_alive;
    csr.index_to_node = d_index_to_node;
    csr.num_nodes = num_nodes;

    constexpr uint32_t block_size = 256;
    uint32_t grid_size = (num_flows + block_size - 1) / block_size;

    simulate_flows_kernel<<<grid_size, block_size>>>(
      csr, d_flow_srcs, d_flow_dests, num_flows, max_hops,
      d_hop_counts, d_failed, d_node_transits);

    cudaGetLastError();
    cudaDeviceSynchronize();

    cudaMemcpy(h_out_hop_counts, d_hop_counts, num_flows * sizeof(uint32_t), cudaMemcpyDeviceToHost);
    cudaMemcpy(h_out_failed, d_failed, num_flows * sizeof(uint8_t), cudaMemcpyDeviceToHost);
    cudaMemcpy(h_out_node_transits, d_node_transits, num_nodes * sizeof(uint32_t), cudaMemcpyDeviceToHost);

    cudaFree(d_row_offsets);
    cudaFree(d_col_indices);
    cudaFree(d_node_alive);
    cudaFree(d_edge_alive);
    cudaFree(d_index_to_node);
    cudaFree(d_flow_srcs);
    cudaFree(d_flow_dests);
    cudaFree(d_hop_counts);
    cudaFree(d_failed);
    cudaFree(d_node_transits);
  }
}
