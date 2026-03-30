#include <iostream>
#include <print>

#include "Helpers/All.hpp"
#include "Rand.hpp"
#include "Topologies/All.hpp"
#include "Routers/All.hpp"
#include "Engines/BasicEngine.hpp"
#include "Topologies/CubeConnectedCycles.hpp"

int main() {
  // using BaseTopo = topo::Hypercube;
  // using BaseTopo = topo::Dragonfly;
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
  
  // BaseTopo graph(4); // hypercube
  // BaseTopo graph(3, 1, 4); // Dragonfly
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

  using BaseTopo = topo::CubeConnectedCycles;
  using CSRTopo = topo::CSRView<BaseTopo>;

  BaseTopo graph(4);

  std::mt19937 rng(42);
  auto csr = topo::build_csr(graph, 0.05, 0.1, rng);

  CSRTopo csr_topo(csr, graph);
  
  engines::BasicEngine<CSRTopo> engine;

  engine.runSim(csr_topo, traffic::gen_rand_traffic<CSRTopo>, route::CCC_next_hop<CSRTopo>);


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
