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

  auto metrics = helper::collect_metrics(engine.finished_flows());
  helper::write_metrics(metrics, sim::config_summary);
  helper::write_utilization(engine.finished_flows(), csr_topo);

  std::println("Simulation complete — wrote results.out");

  helper::check_basic_topology(graph);
  std::cout << "Tests passed!" << std::endl;
  return 0;
}
