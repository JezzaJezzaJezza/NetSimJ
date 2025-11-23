#include <queue>
#include "Helpers/Events.hpp"

namespace engines {

  template <typename Topo>
  class BasicEngine {
    public:
      using Node = typename Topo::node_type;
      using Event = helper::BasicEvents<Node>;
      using Queue = std::priority_queue<Event, std::vector<Event>, helper::EventCompare<Event>>;

      void enqueue(const Event& ev) {
        eventQueue.push(ev);
      }

      template <typename TrafficGen, typename Router>
      void runSim(const Topo& topo, TrafficGen&& traffic_gen, Router&& router) {

        auto flows = traffic_gen(topo);
        
        for(auto& f : flows) {
          eventQueue.push(f);
        }
        
        while(!eventQueue.empty()) {
          // Run the sim
          Event ev = eventQueue.top();
          eventQueue.pop();

          ev.print_node();
          
          Node cur = ev.src;
          Node dest = ev.dest;

          if(cur == dest) continue;

          Node next = router(topo, cur, dest);

          Event nextEv = ev;
          nextEv.src = next;
          nextEv.timestamp += 1;

          eventQueue.push(nextEv);
        }
      }
      
      private:
        Queue eventQueue;
  };
}
