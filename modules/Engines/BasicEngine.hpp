#include <queue>
#include <vector>

#include "Helpers/Events.hpp"

namespace engines {

  template <typename Topo>
  class BasicEngine {
  public:
    using Node  = typename Topo::node_type;
    using Event = helper::BasicEvents<Node>;
    using Queue = std::priority_queue<Event, std::vector<Event>, helper::EventCompare<Event>>;

    void enqueue(const Event& ev) {
      eventQueue.push(ev);
    }

    const std::vector<Event>& finished_flows() const {
      return finished_;
    }

    template <typename TrafficGen, typename Router>
    void runSim(const Topo& topo, TrafficGen&& traffic_gen, Router&& router) {

      auto flows = traffic_gen(topo);


      for (auto& f : flows) {
        f.path.clear();
        f.path.push_back(f.src);
        eventQueue.push(f);
      }

      while (!eventQueue.empty()) {
        Event ev = eventQueue.top();
        eventQueue.pop();

        Node cur  = ev.src;
        Node dest = ev.dest;

        if (cur == dest) {
          finished_.push_back(ev);
          continue;
        }

        Node next = router(topo, cur, dest);

        Event nextEv = ev;
        nextEv.src = next;
        nextEv.timestamp += 1;
        nextEv.path.push_back(next);

        eventQueue.push(nextEv);
      }
    }

  private:
    Queue eventQueue;
    std::vector<Event> finished_;
  };

}
