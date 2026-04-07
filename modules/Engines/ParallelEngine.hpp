#pragma once

#include <vector>
#include <optional>
#include <type_traits>
#include <thread>
#include <algorithm>

#include "Helpers/Events.hpp"

namespace engines {

  template <typename Topo>
  class ParallelEngine {
  public:
    using Node  = typename Topo::node_type;
    using Event = helper::BasicEvents<Node>;

    const std::vector<Event>& finished_flows() const {
      return finished_;
    }

    template <typename TrafficGen, typename Router>
    void runSim(const Topo& topo, TrafficGen&& traffic_gen, Router&& router,
                unsigned num_threads = std::thread::hardware_concurrency()) {
      using RouterResult = std::invoke_result_t<Router, const Topo&, const Node&, const Node&>;

      static_assert(std::is_same_v<RouterResult, std::optional<Node>>, "Router must return std::optional<Node>.");

      if (num_threads == 0) num_threads = 1;

      auto flows = traffic_gen(topo);

      for (auto& f : flows) {
        f.path.clear();
        f.path.push_back(f.src);
        f.failed = false;
      }

      // partition flows across threads
      std::vector<std::vector<Event>> thread_results(num_threads);
      std::vector<std::jthread> threads;
      threads.reserve(num_threads);

      std::size_t total  = flows.size();
      std::size_t chunk  = total / num_threads;
      std::size_t remain = total % num_threads;

      std::size_t offset = 0;
      for (unsigned t = 0; t < num_threads; ++t) {
        std::size_t count = chunk + (t < remain ? 1 : 0);
        std::size_t start = offset;
        std::size_t end   = offset + count;
        offset = end;

        threads.emplace_back([&topo, &router, &flows, &thread_results, t, start, end]() {
          auto& local_finished = thread_results[t];

          for (std::size_t i = start; i < end; ++i) {
            Event ev = flows[i];

            while (true) {
              Node cur  = ev.src;
              Node dest = ev.dest;

              if (cur == dest || ev.failed) {
                local_finished.push_back(ev);
                break;
              }

              std::optional<Node> res = router(topo, cur, dest);

              if (!res) {
                ev.failed = true;
                local_finished.push_back(ev);
                break;
              }

              Node next = *res;
              ev.src = next;
              ev.timestamp += 1;
              ev.path.push_back(next);
            }
          }
        });
      }

      // jthreads join automatically on destruction
      threads.clear();

      // merge results
      std::size_t total_finished = 0;
      for (auto& r : thread_results) total_finished += r.size();
      finished_.reserve(total_finished);

      for (auto& r : thread_results) {
        finished_.insert(finished_.end(),
                         std::make_move_iterator(r.begin()),
                         std::make_move_iterator(r.end()));
      }
    }

  private:
    std::vector<Event> finished_;
  };

}
