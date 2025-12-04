#include "Rand.hpp"
#include "Topologies/All.hpp"
#include "Routers/All.hpp"
#include "Engines/BasicEngine.hpp"

int main() {
  // using Topo = topo::Hypercube;
  using Topo = topo::Dragonfly;
  using Node = Topo::node_type;
  
  // Topo topo(4);
  Topo topo(3, 1, 4);
  engines::BasicEngine<Topo> engine;

  auto flows = traffic::gen_rand_traffic(topo);
  
  
  engine.runSim(topo, traffic::gen_rand_traffic<Topo>, route::DOR_next_hop<Topo>);
  
  return 0;
}
