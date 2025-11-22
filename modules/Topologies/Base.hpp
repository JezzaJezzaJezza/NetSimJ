#include <cstdint>

namespace topo {
  using BitMask = std::uint64_t;  
  template <typename Derived, typename Node>
  class BaseTopo {
    public:
      using node_t = Node;
      
      auto get_neighbours(Node &x) const {
        return static_cast<const Derived*>(this)
          ->template get_neighbours_impl<Node>(x);
      }
  };
}
