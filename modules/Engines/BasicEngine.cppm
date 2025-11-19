#include <deque>
import Events;

namespace engines {

  class BasicEngine {
    
    std::deque<Event> eventQueue;

    public:

      void runSim() {
        while(!eventQueue.empty()) {
          // Run the sim
        }
      }
  };
}
