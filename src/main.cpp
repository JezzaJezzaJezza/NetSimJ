#include <iostream>
#include <print>

#include "Helpers/All.hpp"
#include "Rand.hpp"
#include "Topologies/All.hpp"
#include "Routers/All.hpp"
#include "Engines/BasicEngine.hpp"

int main() {
  using BaseTopo = topo::Hypercube;
  // using Topo = topo::Dragonfly;
  // using Topo = topo::Augmentedcube;
  // using Topo = topo::KaryNcube;
  // using Topo = topo::CrossedCube;
  // using Topo = topo::Zcube;
  // using Topo = topo::Mobiuscube;
  // using Topo = topo::FoldedHypercube;
  // using Topo = topo::ReducedHypercube; // NO DOR INTERFACE (CHANGE DOR ITSELF)
  // using Topo = topo::BalancedHypercube; // NEEDS CUSTOM PRINT INTERFACE
  // using Topo = topo::TwistedCube;
  // using Topo = topo::CubeConnectedCycles;
  
  BaseTopo graph(4); // hypercube
  // Topo topo(3, 1, 4); // Dragonfly
  // Topo topo(4); // Augmented cube
  // Topo topo(5, 7); // K-ary N-cube
  // Topo topo(4); // Crossed cube
  // Topo topo(4); // Zcube
  // Topo topo(4); // Mobius cube
  // Topo topo(4); // Folded Hypercube
  // Topo topo(2, 4); // Reduced Hypercube
  // Topo topo(4); // Balanced Hypercube
  // Topo topo(5); // Twisted cube
  // Topo topo(4); // CCC

  auto csr = topo::build_csr(graph);

  using CSRTopo = topo::CSRView<BaseTopo>;
  CSRTopo csr_topo(csr);
  
  engines::BasicEngine<CSRTopo> engine;

  auto flows = traffic::gen_rand_traffic(graph);
  
  
  engine.runSim(csr_topo, traffic::gen_rand_traffic<CSRTopo>, route::DOR_next_hop<CSRTopo>);

for (const auto& ev : engine.finished_flows()) {
  std::println("Flow {} -> {} | ts = {}",
               graph.node_to_string(ev.path.front()),
               graph.node_to_string(ev.dest),
               ev.timestamp);

  std::print("  path: ");
  for (std::size_t i = 0; i < ev.path.size(); ++i) {
    std::print("{}", graph.node_to_string(ev.path[i]));
    if (i + 1 < ev.path.size()) std::print(" -> ");
  }
  std::println("");
  std::println("");
}
  helper::check_basic_topology(graph);  
  std::cout << "Tests passed!" << std::endl;
  return 0;
}
