#include <cstddef>
#include "Helpers/BitOps.hpp"
#include "Base.hpp"

namespace topo {

  class Hypercube : public BaseTopo<Hypercube, BitMask> {
    private:
      const std::size_t n;
    
    public:
      explicit Hypercube(std::size_t dim) : n(dim) {}
      
      auto get_neighbours_impl(BitMask& x) const {

      }
  };
}

