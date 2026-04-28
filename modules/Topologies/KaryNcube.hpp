#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "Base.hpp"

namespace topo {

  // mixed-radix
  using KaryNode = std::uint64_t;

  class KaryNcube : public BaseTopo<KaryNcube, KaryNode> {
  private:
    const std::size_t k;
    const std::size_t n;
    const std::size_t num_nodes;
    std::size_t* stride;

    static std::size_t int_pow(std::size_t base, std::size_t exp) {
      std::size_t result = 1;
      for (std::size_t i = 0; i < exp; i++) {
        result *= base;
      }
      return result;
    }

    void init_strides() {
      stride = new std::size_t[n];
      std::size_t s = 1;
      for (std::size_t d = 0; d < n; d++) {
        stride[d] = s;
        s *= k;
      }
    }

    std::size_t coord(KaryNode x, std::size_t dim) const {
      return (static_cast<std::size_t>(x) / stride[dim]) % k;
    }

    KaryNode step_forward(KaryNode x, std::size_t dim) const {
      std::size_t c = coord(x, dim);
      if (c + 1 < k) {
        return x + static_cast<KaryNode>(stride[dim]);
      } else {
        return x - static_cast<KaryNode>((k - 1) * stride[dim]);
      }
    }

    KaryNode step_backward(KaryNode x, std::size_t dim) const {
      std::size_t c = coord(x, dim);
      if (c > 0) {
        return x - static_cast<KaryNode>(stride[dim]);
      } else {
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

    KaryNcube(const KaryNcube&) = delete;
    KaryNcube& operator=(const KaryNcube&) = delete;

    std::size_t node_count_impl() const {
      return num_nodes;
    }

    template <typename F>
    void for_each_node_impl(F&& f) const {
      for (std::size_t i = 0; i < num_nodes; i++) {
        f(static_cast<KaryNode>(i));
      }
    }

    template <typename F>
    void for_each_endpoint_impl(F&& f) const {
      for (std::size_t i = 0; i < num_nodes; i++) {
        f(static_cast<KaryNode>(i));
      }
    }

    template <typename F>
    void for_each_neighbour_impl(const KaryNode& x, F&& f) const {
      for (std::size_t dim = 0; dim < n; dim++) {
        f(step_forward(x, dim));
        f(step_backward(x, dim));
      }
    }

    KaryNode neighbour_at_impl(const KaryNode& x, std::size_t i) const {
      const std::size_t deg = degree_impl(x);
      if (i >= deg) {
        throw std::out_of_range("KaryNcube: neighbour index out of range");
      }
      std::size_t dim = i / 2;
      bool forward = (i % 2 == 0);
      return forward ? step_forward(x, dim)
                     : step_backward(x, dim);
    }

    std::size_t degree_impl(const KaryNode&) const {
      return 2 * n;
    }

    std::string node_to_string_impl(KaryNode x) const {
      std::string s;
      s.push_back('(');
      for (std::size_t dim = 0; dim < n; dim++) {
        s += std::to_string(coord(x, dim));
        if (dim + 1 < n) {
          s.push_back(',');
        }
      }
      s.push_back(')');
      return s;
    }
  };
}
