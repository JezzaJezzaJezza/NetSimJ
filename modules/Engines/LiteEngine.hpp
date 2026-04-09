#pragma once

#include <vector>
#include <optional>
#include <type_traits>
#include <thread>
#include <cstdint>
#include <cmath>
#include <limits>
#include <map>
#include <algorithm>

#include "Helpers/Logging.hpp"

namespace engines {

  // Memory-efficient engine for large-scale all-to-all simulations.
  // Instead of materialising every flow and storing full paths, this
  // engine iterates endpoint pairs on-the-fly, simulates each flow
  // inline, and aggregates metrics with O(1) per-flow overhead.
  //
  // Peak memory ≈ CSR graph + endpoint vector + per-thread accumulators.
  template <typename Topo>
  class LiteEngine {
  public:
    using Node = typename Topo::node_type;

    template <typename Router>
    void runSim(const Topo& topo, Router&& router,
                unsigned num_threads = std::thread::hardware_concurrency()) {
      using RouterResult =
        std::invoke_result_t<Router, const Topo&, const Node&, const Node&>;

      static_assert(std::is_same_v<RouterResult, std::optional<Node>>,
                    "Router must return std::optional<Node>.");

      if (num_threads == 0) num_threads = 1;

      // Collect live endpoints once — the only large allocation.
      std::vector<Node> endpoints;
      endpoints.reserve(topo.node_count());
      topo.for_each_endpoint([&](const Node& x) {
        endpoints.push_back(x);
      });

      const std::size_t n        = endpoints.size();
      const std::size_t hop_limit = topo.node_count(); // loop guard

      // ----- per-thread accumulator -----------------------------------
      struct Accum {
        std::size_t total       = 0;
        std::size_t success     = 0;
        std::size_t failed      = 0;
        double      sum_hops    = 0.0;
        double      sum_hops_sq = 0.0;
        std::size_t min_hops    = std::numeric_limits<std::size_t>::max();
        std::size_t max_hops    = 0;
        std::map<std::size_t, std::size_t> hop_hist;
      };

      std::vector<Accum> accums(num_threads);

      // ----- partition source endpoints across threads ----------------
      std::size_t chunk  = n / num_threads;
      std::size_t remain = n % num_threads;

      std::vector<std::jthread> threads;
      threads.reserve(num_threads);

      std::size_t offset = 0;
      for (unsigned t = 0; t < num_threads; ++t) {
        std::size_t count = chunk + (t < remain ? 1 : 0);
        std::size_t start = offset;
        std::size_t end   = offset + count;
        offset = end;

        threads.emplace_back(
          [&topo, &router, &endpoints, &accums, n, hop_limit, t, start, end]() {
            auto& acc = accums[t];

            for (std::size_t si = start; si < end; ++si) {
              const Node& src = endpoints[si];

              for (std::size_t di = 0; di < n; ++di) {
                if (si == di) continue;
                const Node& dest = endpoints[di];

                // --- simulate one flow inline ---
                Node        cur    = src;
                std::size_t hops   = 0;
                bool        failed = false;

                while (cur != dest) {
                  if (hops >= hop_limit) { failed = true; break; }
                  auto next = router(topo, cur, dest);
                  if (!next)            { failed = true; break; }
                  cur = *next;
                  ++hops;
                }

                // --- accumulate ---
                acc.total++;
                if (failed) {
                  acc.failed++;
                } else {
                  acc.success++;
                  double h = static_cast<double>(hops);
                  acc.sum_hops    += h;
                  acc.sum_hops_sq += h * h;
                  if (hops < acc.min_hops) acc.min_hops = hops;
                  if (hops > acc.max_hops) acc.max_hops = hops;
                  acc.hop_hist[hops]++;
                }
              }
            }
          });
      }

      // jthreads join on destruction
      threads.clear();

      // ----- merge accumulators into SimMetrics -----------------------
      helper::SimMetrics<Node> m;

      std::size_t total_success   = 0;
      double      total_sum_hops    = 0.0;
      double      total_sum_hops_sq = 0.0;
      std::size_t global_min = std::numeric_limits<std::size_t>::max();
      std::size_t global_max = 0;
      std::map<std::size_t, std::size_t> global_hist;

      for (auto& acc : accums) {
        m.total_flows  += acc.total;
        m.failed_flows += acc.failed;
        total_success  += acc.success;
        total_sum_hops    += acc.sum_hops;
        total_sum_hops_sq += acc.sum_hops_sq;

        if (acc.success > 0) {
          global_min = std::min(global_min, acc.min_hops);
          global_max = std::max(global_max, acc.max_hops);
        }
        for (auto& [h, c] : acc.hop_hist) global_hist[h] += c;
      }

      m.success_flows = total_success;

      if (total_success > 0) {
        double ns = static_cast<double>(total_success);

        m.avg_hops = total_sum_hops / ns;
        m.min_hops = global_min;
        m.max_hops = global_max;

        double variance = (total_sum_hops_sq / ns) - (m.avg_hops * m.avg_hops);
        m.stddev_hops = std::sqrt(std::max(0.0, variance));

        m.median_hops  = median_from_hist(global_hist, total_success);

        // All flows start at t=0 and each hop adds 1
        m.avg_latency  = m.avg_hops;
        m.max_latency  = global_max;

        m.hop_distribution = std::move(global_hist);
      }

      metrics_ = std::move(m);
    }

    const helper::SimMetrics<Node>& metrics() const { return metrics_; }

  private:
    helper::SimMetrics<Node> metrics_;

    static double median_from_hist(const std::map<std::size_t, std::size_t>& hist,
                                   std::size_t total) {
      auto value_at_rank = [&](std::size_t rank) -> std::size_t {
        std::size_t cumulative = 0;
        for (auto& [val, count] : hist) {
          cumulative += count;
          if (cumulative > rank) return val;
        }
        return 0;
      };

      if (total % 2 == 1) {
        return static_cast<double>(value_at_rank(total / 2));
      }
      std::size_t a = value_at_rank(total / 2 - 1);
      std::size_t b = value_at_rank(total / 2);
      return (static_cast<double>(a) + static_cast<double>(b)) / 2.0;
    }
  };

}
