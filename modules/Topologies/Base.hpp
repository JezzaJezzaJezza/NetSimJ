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

      template <typename F>
      auto for_each_node(F&& f) const {
        static_cast<const Derived*>(this)
          ->for_each_node_impl(std::forward<F>(f));
      }

      template <typename F>
      void for_each_neighbour(const Node& x, F&& f) const {
        static_cast<const Derived*>(this)
          ->for_each_neighbour_impl(x, std::forward<F>(f));
      }

      std::size_t degree(const Node& x) const {
        return static_cast<const Derived*>(this)->degree_impl(x);
      }

      Node neighbour_at(const Node& x, std::size_t i) const {
        return static_cast<const Derived*>(this)->neighbour_at_impl(x);
      }

      template <typename Rand>
      Node random_neighbour(const Node& x, Rand& rng) const {
        auto d = degree(x);
        std::uniform_int_distribution<std::size_t> dist(0, d - 1);
        return neighbour_at(x, dist(rng));
      }

      std::size_t node_count() const {
        return static_cast<const Derived*>(this)->node_count_impl();
      }
  };
}
