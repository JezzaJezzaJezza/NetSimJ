#include <queue>
#include <print>
#include "Traffic/Rand.hpp"
#include "Helpers/Events.hpp"

namespace engines {

  template <typename Topo>
  class BasicEngine {
    public:
      using Node = typename Topo::node_type;
      using Event = helper::BasicEvents<Node>;
      std::priority_queue<Event, std::vector<Event>, helper::EventCompare<Event>> eventQueue;

      void runSim() {
        Topo topo(4); // Change to pass in via ctor

        auto flows = traffic::gen_rand_traffic(topo);

        for(auto& f : flows) {
          eventQueue.push(f);
        }
        
        while(!eventQueue.empty()) {
          // Run the sim
          std::println("Made it here");
          Event ev = eventQueue.top();
          eventQueue.pop();
        }
      }
  };
}
