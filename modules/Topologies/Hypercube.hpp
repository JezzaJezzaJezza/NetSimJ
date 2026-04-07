#include <cstddef>
#include <stdexcept>
#include "Base.hpp"

namespace topo {
  
  using BitMask = std::uint64_t;  

  class Hypercube : public BaseTopo<Hypercube, BitMask> {
    private:
      const std::size_t n;
      const std::size_t num_nodes;
    
    public:
      
      using node_type = BitMask;
      
      explicit Hypercube(std::size_t dim) : n(dim), num_nodes(node_count_impl()) {
        if(n > sizeof(BitMask) * 8) {
          throw std::runtime_error("Dimension too large for BitMask type");
        }
      }

      std::size_t node_count_impl() const {
        return std::size_t{1} << n;
      }

      template <typename F>
      void for_each_node_impl(F&& f) const {
        for(std::size_t i = 0; i < num_nodes; i++) {
          BitMask x = static_cast<BitMask>(i);
          f(x);
        }
      }

      template <typename F>
      void for_each_endpoint_impl(F&& f) const {
        for_each_node_impl(std::forward<F>(f));
      }

      template <typename F>
      void for_each_neighbour_impl(const BitMask& x, F&& f) const {
        for(std::size_t i = 0; i < n; i++) {
          BitMask mask = BitMask{1} << i;
          BitMask neighbour = x ^ mask;
          f(neighbour);
        }
      }

      std::size_t dim() const { return n; }

      std::size_t degree_impl(const BitMask&) const {
        return n;
      }

      BitMask neighbour_at_impl(const BitMask& x, std::size_t i) const {
        BitMask mask = BitMask{1} << i;
        return x ^ mask;
      }

      std::string node_to_string_impl(BitMask x) const {
        std::string s;
        s.reserve(n);

        for(int i = static_cast<int>(n) - 1; i >= 0; i--) {
          BitMask mask = BitMask{1} << i;
          s.push_back((x & mask) ? '1' : '0');
        }

        return s;
      }

  };
}
