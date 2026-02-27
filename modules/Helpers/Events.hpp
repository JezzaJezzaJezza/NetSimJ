#pragma once
#include <cstdint>
#include <print>
#include <vector>

namespace helper {

  template <typename Node>
  class BasicEvents {
    public:
      Node src;
      Node dest;
      std::uint64_t timestamp = 0;

      std::vector<Node> path;

      void print_node() const {
        std::println("Event(src = {}, dest = {}, ts = {})", src, dest, timestamp);
      }
  };
    
  template<typename Event>
  struct EventCompare {
    bool operator()(const Event& a, const Event& b) const {
      return a.timestamp > b.timestamp;
    }
  };
}
