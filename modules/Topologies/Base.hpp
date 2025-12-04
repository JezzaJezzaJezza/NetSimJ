#pragma once

#include <random>
#include <utility>

namespace topo {
  template <typename Derived, typename Node>
  class BaseTopo {
    public:
      using node_type = Node;
      
      auto get_neighbours(Node &x) const {
        return static_cast<const Derived*>(this)
          ->template get_neighbours_impl<Node>(x);
      }

      // Method to iterate over all nodes in any network
      template <typename F>
      auto for_each_node(F&& f) const {
        static_cast<const Derived*>(this)
          ->for_each_node_impl(std::forward<F>(f));
      }

      // Method to iterate over all neighbours of any given node
      template <typename F>
      void for_each_neighbour(const Node& x, F&& f) const {
        static_cast<const Derived*>(this)
          ->for_each_neighbour_impl(x, std::forward<F>(f));
      }

      // Get the degree of some specified node
      std::size_t degree(const Node& x) const {
        return static_cast<const Derived*>(this)->degree_impl(x);
      }

      // Get the i'th neighbour of some node
      Node neighbour_at(const Node& x, std::size_t i) const {
        return static_cast<const Derived*>(this)->neighbour_at_impl(x, i);
      }

      // Get a random neighbour for some node
      template <typename Rand>
      Node random_neighbour(const Node& x, Rand& rng) const {
        auto d = degree(x);
        std::uniform_int_distribution<std::size_t> dist(0, d - 1);
        return neighbour_at(x, dist(rng));
      }

      // Count the total number of nodes (exclusively endpoints)
      std::size_t node_count() const {
        return static_cast<const Derived*>(this)->node_count_impl();
      }
  };
}
