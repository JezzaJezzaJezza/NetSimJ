#include <iostream>
#include "Helpers/All.hpp"
#include "Rand.hpp"
#include "Topologies/All.hpp"
#include "Routers/All.hpp"
#include "Engines/BasicEngine.hpp"

int main() {
  using Topo = topo::Hypercube;
  // using Topo = topo::Dragonfly;
  // using Topo = topo::Augmentedcube;
  // using Topo = topo::KaryNcube;
  // using Topo = topo::CrossedCube;
  // using Topo = topo::Zcube;
  // using Topo = topo::Mobiuscube;
  // using Topo = topo::FoldedHypercube;
  using Node = Topo::node_type;
  
  // Topo topo(4); // hypercube
  // Topo topo(3, 1, 4); // Dragonfly
  // Topo topo(4); // Augmented cube
  // Topo topo(4, 5); // K-ary N-cube
  // Topo topo(4); // Crossed cube
  // Topo topo(4); // Zcube
  // Topo topo(4); // Mobius cube
  Topo topo(4); // Folded Hyper cube
  engines::BasicEngine<Topo> engine;

  auto flows = traffic::gen_rand_traffic(topo);
  
  
  engine.runSim(topo, traffic::gen_rand_traffic<Topo>, route::DOR_next_hop<Topo>);


  helper::check_basic_topology(topo);  
  std::cout << "Tests passed!" << std::endl;
  return 0;
}
