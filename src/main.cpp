#include <print>

#include "SimConfig.hpp"
#include "Helpers/All.hpp"

int main() {
  auto graph = sim::make_topology();

  std::mt19937 rng(sim::rng_seed);
  auto csr = topo::build_csr(graph, sim::node_fault_prob, sim::edge_fault_prob, rng);
  sim::CSRTopo csr_topo(csr, graph);

#if SIM_USE_CUDA
  sim::Engine engine;
  engine.runSim(csr, csr_topo, sim::gen_traffic<sim::CSRTopo>);

  auto metrics = helper::collect_metrics(engine.finished_flows());
  helper::write_metrics(metrics, sim::config_summary);
#elif SIM_LITE_ENGINE
  sim::Engine engine;
  engine.runSim(csr_topo, sim::next_hop<sim::CSRTopo>);

  const auto& metrics = engine.metrics();
  helper::write_metrics(metrics, sim::config_summary);
#else
  sim::Engine engine;
  engine.runSim(csr_topo, sim::gen_traffic<sim::CSRTopo>, sim::next_hop<sim::CSRTopo>);

  auto metrics = helper::collect_metrics(engine.finished_flows());
  helper::write_metrics(metrics, sim::config_summary);
  helper::write_util(engine.finished_flows(), csr_topo);
#endif

  std::println("Simulation complete — wrote results.out");
  return 0;
}
