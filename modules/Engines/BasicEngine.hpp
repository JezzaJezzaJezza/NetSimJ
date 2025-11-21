#include <queue>
import Events;

namespace engines {

  class BasicEngine {
    
    std::priority_queue<Event> eventQueue;

    public:

      void runSim() {
        while(!eventQueue.empty()) {
          // Run the sim
        }
      }
  };
}
