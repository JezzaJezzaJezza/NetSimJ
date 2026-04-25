#pragma once

#include <queue>
#include <vector>
#include <optional>
#include <type_traits>

#include "Helpers/Events.hpp"

namespace engines {

  template <typename Topo>
  class BasicEngine {
  public:
    using Node  = typename Topo::node_type;
    using Event = helper::BasicEvents<Node>;
    using Queue = std::priority_queue<Event, std::vector<Event>, helper::EventCompare<Event>>; // use for congestion engines later

    void enqueue(const Event& ev) {
      eventQueue.push(ev);
    }

    const std::vector<Event>& finished_flows() const {
      return finished_;
    }

    template <typename TrafficGen, typename Router>
    void runSim(const Topo& topo, TrafficGen&& traffic_gen, Router&& router) {
      using RouterResult = std::invoke_result_t<Router, const Topo&, const Node&, const Node&>;

      static_assert(std::is_same_v<RouterResult, std::optional<Node>>, "Router must return std::optional<Node>.");

      auto flows = traffic_gen(topo);

      // init flows
      for (auto& f : flows) {
        f.path.clear();
        f.path.push_back(f.src);
        f.failed = false;
        eventQueue.push(f);
      }

      while (!eventQueue.empty()) {
        Event ev = eventQueue.top();
        eventQueue.pop();

        Node cur  = ev.src;
        Node dest = ev.dest;

        if (cur == dest || ev.failed) {
          finished_.push_back(ev);
          continue;
        }

        std::optional<Node> res = router(topo, cur, dest);

        if (!res) {
          ev.failed = true;
          finished_.push_back(ev);
          continue;
        }

        Node next = *res;

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
