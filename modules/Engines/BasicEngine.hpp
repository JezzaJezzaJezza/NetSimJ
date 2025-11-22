#include <queue>
#include "Topologies/Hypercube.hpp"

namespace engines {

  class BasicEngine {
    
    std::priority_queue<int> eventQueue;

    public:

      void runSim() {
        while(!eventQueue.empty()) {
          // Run the sim
        }
      }
  };
}
