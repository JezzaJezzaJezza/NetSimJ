#include <iostream>
#include <print>

#include "SimConfig.hpp"
#include "Helpers/All.hpp"
#include "Engines/BasicEngine.hpp"

int main() {
  auto graph = sim::make_topology();

  std::mt19937 rng(sim::rng_seed);
  auto csr = topo::build_csr(graph, sim::node_fault_prob, sim::edge_fault_prob, rng);

  sim::CSRTopo csr_topo(csr, graph);
  engines::BasicEngine<sim::CSRTopo> engine;

  engine.runSim(csr_topo, sim::gen_traffic<sim::CSRTopo>, sim::next_hop<sim::CSRTopo>);

  for (const auto& ev : engine.finished_flows()) {
    std::print("Flow {} -> {} | path: ",
               csr_topo.node_to_string(ev.path.front()),
               csr_topo.node_to_string(ev.dest));

    for (std::size_t i = 0; i < ev.path.size(); ++i) {
      std::print("{}", csr_topo.node_to_string(ev.path[i]));
      if (i + 1 < ev.path.size()) std::print(" -> ");
    }

    if (ev.failed) {
      std::print(" -> FAILED");
    }
    std::println("\n");
  }

  helper::check_basic_topology(graph);
  std::cout << "Tests passed!" << std::endl;
  return 0;
}
