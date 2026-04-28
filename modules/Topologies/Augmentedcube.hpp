#include <cstddef>
#include <stdexcept>
#include "Base.hpp"

namespace topo {
  
  using BitMask = std::uint64_t;  

  class Augmentedcube: public BaseTopo<Augmentedcube, BitMask> {
    private:
      const std::size_t n;
      const std::size_t num_nodes;
    
    public:
      
      using node_type = BitMask;
      
      explicit Augmentedcube(std::size_t dim) : n(dim), num_nodes(node_count_impl()) {
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
          f(static_cast<BitMask>(i));
        }
      }

      template <typename F>
      void for_each_endpoint_impl(F&& f) const {
        for_each_node_impl(std::forward<F>(f));
      }

      template <typename F>
      void for_each_neighbour_impl(const BitMask& x, F&& f) const {
        if (n == 0) return;

        BitMask mask_comp0 = BitMask{1};
        BitMask neighbour = x ^ mask_comp0;
        f(neighbour);

        for (std::size_t j = 1; j < n; j++) {
          BitMask mask_norm = BitMask{1} << j;
          BitMask neighbour_norm = x ^ mask_norm;
          f(neighbour_norm);

          BitMask mask_comp = (BitMask{1} << (j + 1)) - 1;
          BitMask neighbour_comp = x ^ mask_comp;
          f(neighbour_comp);
        }
      }

      std::size_t degree_impl(const BitMask&) const {
        if (n == 0) return 0;
        return 2 * n - 1;
      }

      BitMask neighbour_at_impl(const BitMask& x, std::size_t i) const {
        const std::size_t deg = degree_impl(x);
        if (i >= deg) {
          throw std::out_of_range("Augmented Cube: Neighbour index out of range (neighbour_at_impl)");
        }

        if (i == 0) {
          BitMask mask_comp0 = BitMask{1};
          return x ^ mask_comp0;
        }

        std::size_t j = (i + 1) / 2;

        if (j >= n) {
          throw std::logic_error("Augmented Cube: Bad index mapping (neighbour_at_impl)");
        }

        if (i % 2 == 1) {
          BitMask mask_norm = BitMask{1} << j;
          return x ^ mask_norm;
        } else {
          BitMask mask_comp = (BitMask{1} << (j + 1)) - 1;
          return x ^ mask_comp;
        }
      }

      std::size_t dim_count() const {
        return n;
      }
      
      bool dim_aligned(BitMask a, BitMask b, std::size_t dim) const {
        BitMask mask = BitMask{1} << dim;
        return ((a ^ b) & mask) == 0;
      }

      BitMask move_to(BitMask from, BitMask to, std::size_t dim) const {
        BitMask mask = BitMask{1} << dim;

        if(((from ^ to) & mask) != 0) {
          return from ^ mask;
        }
        return from;
      }

      std::string node_to_string_impl(BitMask x) const {
        std::string s;
        s.reserve(n);
        for (int i = static_cast<int>(n) - 1; i >= 0; i--) {
          s.push_back((x >> i) & 1 ? '1' : '0');
        }
        return s;
      }
  };
}

