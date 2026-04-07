#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <map>
#include <numeric>
#include <string>
#include <vector>

#include "Events.hpp"

namespace helper {

  template <typename Node>
  struct SimMetrics {
    std::size_t total_flows    = 0;
    std::size_t success_flows  = 0;
    std::size_t failed_flows   = 0;

    // Hop counts (successful flows only)
    double      avg_hops       = 0.0;
    std::size_t min_hops       = 0;
    std::size_t max_hops       = 0;
    double      median_hops    = 0.0;
    double      stddev_hops    = 0.0;

    // Latency (timestamp at completion, successful flows only)
    double      avg_latency    = 0.0;
    std::size_t max_latency    = 0;

    // Per-flow hop counts for distribution
    std::vector<std::size_t> hop_counts;
  };

  template <typename Node>
  SimMetrics<Node> collect_metrics(const std::vector<BasicEvents<Node>>& flows) {
    SimMetrics<Node> m;
    m.total_flows = flows.size();

    for (const auto& f : flows) {
      if (f.failed) {
        m.failed_flows++;
        continue;
      }
      m.success_flows++;

      // hops = edges traversed = path length - 1
      std::size_t hops = f.path.size() > 0 ? f.path.size() - 1 : 0;
      m.hop_counts.push_back(hops);
    }

    if (m.hop_counts.empty()) return m;

    // Sort for median
    std::sort(m.hop_counts.begin(), m.hop_counts.end());

    m.min_hops = m.hop_counts.front();
    m.max_hops = m.hop_counts.back();

    double sum = std::accumulate(m.hop_counts.begin(), m.hop_counts.end(), 0.0);
    double n   = static_cast<double>(m.hop_counts.size());
    m.avg_hops = sum / n;

    // Median
    std::size_t mid = m.hop_counts.size() / 2;
    if (m.hop_counts.size() % 2 == 0) {
      m.median_hops = (m.hop_counts[mid - 1] + m.hop_counts[mid]) / 2.0;
    } else {
      m.median_hops = static_cast<double>(m.hop_counts[mid]);
    }

    // Standard deviation
    double sq_sum = 0.0;
    for (auto h : m.hop_counts) {
      double diff = static_cast<double>(h) - m.avg_hops;
      sq_sum += diff * diff;
    }
    m.stddev_hops = std::sqrt(sq_sum / n);

    // Latency from successful flows
    double lat_sum = 0.0;
    m.max_latency  = 0;
    for (const auto& f : flows) {
      if (f.failed) continue;
      auto lat = static_cast<std::size_t>(f.timestamp);
      lat_sum += static_cast<double>(lat);
      if (lat > m.max_latency) m.max_latency = lat;
    }
    m.avg_latency = lat_sum / n;

    return m;
  }

  template <typename Node>
  void write_metrics(const SimMetrics<Node>& m,
                     const std::string& config_summary,
                     const std::string& filename = "results.out") {
    std::ofstream out(filename);

    out << "=== NetSimJ Simulation Results ===\n\n";
    out << config_summary << "\n\n";

    // Flow summary
    out << "--- Flow Summary ---\n";
    out << "Total flows      : " << m.total_flows    << "\n";
    out << "Successful       : " << m.success_flows   << "\n";
    out << "Failed           : " << m.failed_flows    << "\n";
    if (m.total_flows > 0) {
      double rate = 100.0 * static_cast<double>(m.success_flows)
                          / static_cast<double>(m.total_flows);
      out << "Success rate     : " << rate << "%\n";
    }

    out << "\n--- Hop Count (successful flows) ---\n";
    if (m.hop_counts.empty()) {
      out << "No successful flows.\n";
    } else {
      out << "Min              : " << m.min_hops     << "\n";
      out << "Max              : " << m.max_hops     << "\n";
      out << "Average          : " << m.avg_hops     << "\n";
      out << "Median           : " << m.median_hops  << "\n";
      out << "Std deviation    : " << m.stddev_hops  << "\n";
    }

    out << "\n--- Latency (successful flows) ---\n";
    if (m.hop_counts.empty()) {
      out << "No successful flows.\n";
    } else {
      out << "Avg latency      : " << m.avg_latency  << "\n";
      out << "Max latency      : " << m.max_latency  << "\n";
    }

    // Hop distribution
    if (!m.hop_counts.empty()) {
      out << "\n--- Hop Distribution ---\n";

      std::size_t max_hop = m.hop_counts.back();
      std::vector<std::size_t> dist(max_hop + 1, 0);
      for (auto h : m.hop_counts) dist[h]++;

      for (std::size_t i = 0; i <= max_hop; ++i) {
        if (dist[i] > 0) {
          out << "  " << i << " hops : " << dist[i] << "\n";
        }
      }
    }

    out << "\n";
  }

  // Count how many flows transit through each node (intermediate hops only).
  // Surfaces choke points: nodes that many flows are forced through.
  template <typename Topo>
  void write_utilization(const std::vector<BasicEvents<typename Topo::node_type>>& flows,
                         const Topo& topo,
                         const std::string& filename = "results.out") {
    using Node = typename Topo::node_type;

    // node label -> transit count
    std::map<std::string, std::size_t> transit_counts;

    // Seed with all live nodes so we can report unused ones
    topo.for_each_node([&](const Node& n) {
      transit_counts[topo.node_to_string(n)] = 0;
    });

    std::size_t total_transits = 0;

    for (const auto& f : flows) {
      if (f.failed || f.path.size() < 3) continue;

      // Intermediate nodes only (skip src at [0] and dest at [back])
      for (std::size_t i = 1; i + 1 < f.path.size(); ++i) {
        transit_counts[topo.node_to_string(f.path[i])]++;
        total_transits++;
      }
    }

    // Sort nodes by transit count descending
    std::vector<std::pair<std::string, std::size_t>> ranked(
      transit_counts.begin(), transit_counts.end());

    std::sort(ranked.begin(), ranked.end(),
      [](const auto& a, const auto& b) { return a.second > b.second; });

    // Gather counts for stats
    std::vector<std::size_t> counts;
    counts.reserve(ranked.size());
    for (const auto& [_, c] : ranked) counts.push_back(c);

    std::size_t nodes_used   = 0;
    std::size_t nodes_unused = 0;
    for (auto c : counts) {
      if (c > 0) nodes_used++;
      else       nodes_unused++;
    }

    double avg_transit = 0.0;
    double stddev      = 0.0;
    if (!counts.empty()) {
      double sum = std::accumulate(counts.begin(), counts.end(), 0.0);
      double n   = static_cast<double>(counts.size());
      avg_transit = sum / n;

      double sq_sum = 0.0;
      for (auto c : counts) {
        double diff = static_cast<double>(c) - avg_transit;
        sq_sum += diff * diff;
      }
      stddev = std::sqrt(sq_sum / n);
    }

    // Append to the results file
    std::ofstream out(filename, std::ios::app);

    out << "--- Node Utilization (transit only) ---\n";
    out << "Total transits   : " << total_transits << "\n";
    out << "Nodes in network : " << counts.size()  << "\n";
    out << "Nodes traversed  : " << nodes_used     << "\n";
    out << "Nodes unused     : " << nodes_unused   << "\n";

    if (!counts.empty() && counts.front() > 0) {
      out << "Avg transits     : " << avg_transit << "\n";
      out << "Std deviation    : " << stddev      << "\n";

      std::size_t max_transit = counts.front();
      double ratio = (avg_transit > 0.0)
                   ? static_cast<double>(max_transit) / avg_transit
                   : 0.0;
      out << "Max transits     : " << max_transit << "\n";
      out << "Max/Avg ratio    : " << ratio
          << (ratio > 2.0 ? "  (high — potential bottleneck)" : "") << "\n";
    }

    // Top choke points
    std::size_t top_n = std::min<std::size_t>(10, ranked.size());
    out << "\nTop " << top_n << " choke points:\n";
    for (std::size_t i = 0; i < top_n; ++i) {
      if (ranked[i].second == 0) break;
      out << "  " << (i + 1) << ". " << ranked[i].first
          << " : " << ranked[i].second << " transits\n";
    }

    out << "\n";
  }

}
