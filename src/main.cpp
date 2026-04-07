#include <iostream>
#include <print>

#include "SimConfig.hpp"
#include "Helpers/All.hpp"

int main() {
  auto graph = sim::make_topology();

#if SIM_USE_CUDA
  sim::Engine engine;
  engine.runSim(graph, sim::gen_traffic<sim::BaseTopo>, sim::next_hop<sim::BaseTopo>);

  auto metrics = helper::collect_metrics(engine.finished_flows());
  helper::write_metrics(metrics, sim::config_summary);
#else
  std::mt19937 rng(sim::rng_seed);
  auto csr = topo::build_csr(graph, sim::node_fault_prob, sim::edge_fault_prob, rng);

  sim::CSRTopo csr_topo(csr, graph);
  sim::Engine engine;

  engine.runSim(csr_topo, sim::gen_traffic<sim::CSRTopo>, sim::next_hop<sim::CSRTopo>);

  auto metrics = helper::collect_metrics(engine.finished_flows());
  helper::write_metrics(metrics, sim::config_summary);
  helper::write_utilization(engine.finished_flows(), csr_topo);

  helper::check_basic_topology(graph);
  std::cout << "Tests passed!" << std::endl;
#endif

  std::println("Simulation complete — wrote results.out");
  return 0;
}
