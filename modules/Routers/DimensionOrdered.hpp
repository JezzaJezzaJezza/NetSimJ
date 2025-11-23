#include <vector>

namespace route {
  template <typename Topo>
  std::vector<typename Topo::node_type> DOR(const Topo& topo,
                                            typename Topo::node_type src,
                                            typename Topo::node_type dest) {
    using Node = typename Topo::node_type;

    std::vector<Node> path;
    path.push_back(src);

    Node cur = src;
    const std::size_t D = topo.dim_count();

    for(std::size_t dim = 0; dim < D; dim++) {
      while(!topo.dim_aligned(cur, dest, dim)) {
        cur = topo.move_To(cur, dest, dim);
        path.push_back(cur);
      }
    }
  }

  template <typename Topo>
  typename Topo::node_type DOR_next_hop(const Topo& topo,
                                        const typename Topo::node_type& cur,
                                        const typename Topo::node_type& dest) {
    using Node = typename Topo::node_type;

    const std::size_t D = topo.dim_count();

    Node next = cur;
    for(std::size_t dim = 0; dim < D; dim++) {
      if(!topo.dim_aligned(cur, dest, dim)) {
        next = topo.move_to(cur, dest, dim);
        break;
      }
    }
    return next;
  }

}
