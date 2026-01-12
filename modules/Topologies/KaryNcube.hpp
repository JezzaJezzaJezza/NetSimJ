#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include "Base.hpp"

namespace topo {


// GPT Optimised (used non-tuple structure approach - honestly have no idea how it managed it)

  using KaryNode = std::uint64_t;

  class KaryNcube : public BaseTopo<KaryNcube, KaryNode> {
  private:
    const std::size_t k;          // nodes per dimension
    const std::size_t n;          // number of dimensions
    const std::size_t num_nodes;  // k^n
    std::size_t* stride;          // length n, stride[d] = k^d

    static std::size_t int_pow(std::size_t base, std::size_t exp) {
      std::size_t result = 1;
      for (std::size_t i = 0; i < exp; ++i) {
        result *= base;
      }
      return result;
    }

    void init_strides() {
      stride = new std::size_t[n];
      std::size_t s = 1;
      for (std::size_t d = 0; d < n; ++d) {
        stride[d] = s;
        s *= k;
      }
    }

    // coordinate of node x in dimension dim (0 .. k-1)
    std::size_t coord(KaryNode x, std::size_t dim) const {
      return (static_cast<std::size_t>(x) / stride[dim]) % k;
    }

    // one step +1 in dimension dim (wrap-around)
    KaryNode step_forward(KaryNode x, std::size_t dim) const {
      std::size_t c = coord(x, dim);
      if (c + 1 < k) {
        return x + static_cast<KaryNode>(stride[dim]);
      } else {
        // wrap: k-1 -> 0
        return x - static_cast<KaryNode>((k - 1) * stride[dim]);
      }
    }

    // one step -1 in dimension dim (wrap-around)
    KaryNode step_backward(KaryNode x, std::size_t dim) const {
      std::size_t c = coord(x, dim);
      if (c > 0) {
        return x - static_cast<KaryNode>(stride[dim]);
      } else {
        // wrap: 0 -> k-1
        return x + static_cast<KaryNode>((k - 1) * stride[dim]);
      }
    }

  public:
    using node_type = KaryNode;

    explicit KaryNcube(std::size_t k_ary, std::size_t dims)
      : k(k_ary),
        n(dims),
        num_nodes(int_pow(k_ary, dims)),
        stride(nullptr)
    {
      if (k < 2) {
        throw std::invalid_argument("KaryNcube: k must be >= 2");
      }
      if (n == 0) {
        throw std::invalid_argument("KaryNcube: dims must be > 0");
      }
      init_strides();
    }

    ~KaryNcube() {
      delete[] stride;
    }

    // no copying; keeps the raw pointer life simple
    KaryNcube(const KaryNcube&) = delete;
    KaryNcube& operator=(const KaryNcube&) = delete;

    // total nodes = k^n
    std::size_t node_count_impl() const {
      return num_nodes;
    }

    template <typename F>
    void for_each_node_impl(F&& f) const {
      for (std::size_t i = 0; i < num_nodes; ++i) {
        f(static_cast<KaryNode>(i));
      }
    }

    // classic k-ary n-cube (torus): 2 neighbours per dimension (+1 and -1)
    template <typename F>
    void for_each_neighbour_impl(const KaryNode& x, F&& f) const {
      for (std::size_t dim = 0; dim < n; ++dim) {
        f(step_forward(x, dim));
        f(step_backward(x, dim));
      }
    }

    std::size_t degree_impl(const KaryNode&) const {
      return 2 * n;
    }

    // neighbour ordering: [dim0 +, dim0 -, dim1 +, dim1 -, ...]
    KaryNode neighbour_at_impl(const KaryNode& x, std::size_t i) const {
      const std::size_t deg = degree_impl(x);
      if (i >= deg) {
        throw std::out_of_range("KaryNcube: neighbour index out of range");
      }

      std::size_t dim = i / 2;
      bool forward = (i % 2 == 0);
      return forward ? step_forward(x, dim) : step_backward(x, dim);
    }

    // DOR interface – same spirit as hypercube: one dimension per coordinate
    std::size_t dim_count() const {
      return n;
    }

    // aligned iff coordinate in that dimension is equal
    bool dim_aligned(KaryNode a, KaryNode b, std::size_t dim) const {
      return coord(a, dim) == coord(b, dim);
    }

    // Dumb, canonical move: if coord differs, take a +1 step in that dimension.
    // If some fancy router wants non-minimal behavior, it should ignore this
    // and use for_each_neighbour_impl instead.
    KaryNode move_to(KaryNode from, KaryNode to, std::size_t dim) const {
      if (coord(from, dim) == coord(to, dim)) {
        return from;
      }
      return step_forward(from, dim);
    }
  };

}
