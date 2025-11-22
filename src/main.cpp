#include <print>
#include "Topologies/All.hpp"
#include "Engines/BasicEngine.hpp"

int main() {
  using Topo = topo::Hypercube;

  engines::BasicEngine<Topo> engine;
  engine.runSim();
  
  std::println("Test");  
  return 1;
}
