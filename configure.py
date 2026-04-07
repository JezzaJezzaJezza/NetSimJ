#!/usr/bin/env python3
"""Configure NetSimJ simulation parameters and generate src/SimConfig.hpp."""

import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT = os.path.join(SCRIPT_DIR, "src", "SimConfig.hpp")

# (name, C++ class, header, [(param_name, description, default)], router or None)
TOPOLOGIES = [
    ("Hypercube",           "topo::Hypercube",            "Topologies/Hypercube.hpp",            [("dim", "Dimension", 4)],                                                      "DOR"),
    ("Augmentedcube",       "topo::Augmentedcube",        "Topologies/Augmentedcube.hpp",        [("dim", "Dimension", 4)],                                                      "DOR"),
    ("CrossedCube",         "topo::CrossedCube",          "Topologies/Crossedcube.hpp",          [("dim", "Dimension", 4)],                                                      "DOR"),
    ("Zcube",               "topo::Zcube",                "Topologies/Zcube.hpp",                [("dim", "Dimension", 4)],                                                      "DOR"),
    ("Mobiuscube",          "topo::Mobiuscube",           "Topologies/Mobiuscube.hpp",           [("dim", "Dimension", 4)],                                                      "DOR"),
    ("FoldedHypercube",     "topo::FoldedHypercube",      "Topologies/FoldedHypercube.hpp",      [("dim", "Dimension", 4)],                                                      "DOR"),
    ("TwistedCube",         "topo::TwistedCube",          "Topologies/Twistedcube.hpp",          [("dim", "Dimension", 5)],                                                      "DOR"),
    ("Paritycube",          "topo::TwistedCubeConnected", "Topologies/Paritycube.hpp",           [("dim", "Dimension", 4)],                                                      "DOR"),
    ("ReducedHypercube",    "topo::ReducedHypercube",     "Topologies/ReducedHypercube.hpp",     [("k", "k (field size)", 2), ("n", "n (sub-dim)", 4)],                           None),
    ("CubeConnectedCycles", "topo::CubeConnectedCycles",  "Topologies/CubeConnectedCycles.hpp",  [("dim", "Dimension", 4)],                                                      "CCC"),
    ("KaryNcube",           "topo::KaryNcube",            "Topologies/KaryNcube.hpp",            [("k", "k (radix)", 5), ("n", "n (dimensions)", 7)],                             None),
    ("BalancedHypercube",   "topo::BalancedHypercube",    "Topologies/BalancedHypercube.hpp",    [("dim", "Dimension", 4)],                                                      None),
    ("Dragonfly",           "topo::Dragonfly",            "Topologies/Dragonfly.hpp",            [("g", "Groups", 3), ("s", "Switches/group", 1), ("e", "Endpoints/switch", 4)],  None),
]

ENGINES = [
    ("BasicEngine",    "engines::BasicEngine",    "Engines/BasicEngine.hpp"),
    ("ParallelEngine", "engines::ParallelEngine",  "Engines/ParallelEngine.hpp"),
]

ROUTER_HEADER = {
    "DOR": "Routers/DimensionOrdered.hpp",
    "CCC": "Routers/CCCRouting.hpp",
}

ROUTER_BODY = {
    "DOR": """\
  template <typename T>
  std::optional<typename T::node_type>
  next_hop(const T& topo, const typename T::node_type& cur, const typename T::node_type& dest) {
    return route::hypercube_DOR(topo, cur, dest);
  }""",
    "CCC": """\
  template <typename T>
  std::optional<typename T::node_type>
  next_hop(const T& topo, const typename T::node_type& cur, const typename T::node_type& dest) {
    return route::CCC_next_hop(topo, cur, dest);
  }""",
}


def prompt_int(msg, default):
    raw = input(f"  {msg} [{default}]: ").strip()
    if not raw:
        return default
    return int(raw)


def prompt_float(msg, default):
    raw = input(f"  {msg} [{default}]: ").strip()
    if not raw:
        return default
    return float(raw)


def main():
    print("\n=== NetSimJ Configuration ===\n")

    # --- Topology ---
    print("Available topologies:")
    for i, (name, _, _, _, router) in enumerate(TOPOLOGIES, 1):
        note = "" if router else "  (no routing implemented yet)"
        print(f"  {i:2d}. {name}{note}")
    print()

    while True:
        raw = input(f"Select topology [1-{len(TOPOLOGIES)}]: ").strip()
        try:
            idx = int(raw) - 1
            if 0 <= idx < len(TOPOLOGIES):
                if TOPOLOGIES[idx][4] is None:
                    print(f"  {TOPOLOGIES[idx][0]} has no routing implemented yet, pick another.")
                    continue
                break
        except ValueError:
            pass
        print("  Invalid choice, try again.")

    name, cpp_class, topo_header, params, router = TOPOLOGIES[idx]
    print(f"\n  -> {name} (router: {router})\n")

    # --- Topology parameters ---
    param_values = []
    for _, desc, default in params:
        param_values.append(prompt_int(desc, default))

    # --- Fault injection ---
    print()
    node_fp = prompt_float("Node fault probability", 0.05)
    edge_fp = prompt_float("Edge fault probability", 0.10)
    seed = prompt_int("RNG seed", 42)

    # --- Engine ---
    print("\nAvailable engines:")
    for i, (ename, _, _) in enumerate(ENGINES, 1):
        print(f"  {i:2d}. {ename}")
    print()

    while True:
        raw = input(f"Select engine [1-{len(ENGINES)}] [1]: ").strip()
        if not raw:
            eng_idx = 0
            break
        try:
            eng_idx = int(raw) - 1
            if 0 <= eng_idx < len(ENGINES):
                break
        except ValueError:
            pass
        print("  Invalid choice, try again.")

    eng_name, eng_class, eng_header = ENGINES[eng_idx]
    print(f"\n  -> {eng_name}\n")

    # --- Generate header ---
    ctor_args = ", ".join(str(v) for v in param_values)

    header = f"""\
#pragma once
// Generated by configure.py — do not edit manually.
//
// Topology : {name}({ctor_args})
// Router   : {router}
// Engine   : {eng_name}
// Faults   : node={node_fp}, edge={edge_fp}, seed={seed}

#include <optional>
#include <random>
#include <string>
#include "{topo_header}"
#include "Topologies/CSR.hpp"
#include "Topologies/CSRView.hpp"
#include "{ROUTER_HEADER[router]}"
#include "{eng_header}"
#include "Traffic/Rand.hpp"

namespace sim {{
  // Topology
  using BaseTopo = {cpp_class};
  using CSRTopo  = topo::CSRView<BaseTopo>;
  using Engine   = {eng_class}<CSRTopo>;

  inline BaseTopo make_topology() {{ return BaseTopo({ctor_args}); }}

  // Fault injection
  inline constexpr unsigned rng_seed        = {seed};
  inline constexpr double   node_fault_prob = {node_fp};
  inline constexpr double   edge_fault_prob = {edge_fp};

  // Config summary for logging
  inline const std::string config_summary =
    "Topology : {name}({ctor_args})\\n"
    "Router   : {router}\\n"
    "Engine   : {eng_name}\\n"
    "Faults   : node={node_fp}, edge={edge_fp}, seed={seed}";

  // Router
{ROUTER_BODY[router]}

  // Traffic
  template <typename T>
  auto gen_traffic(const T& topo) {{ return traffic::gen_rand_traffic(topo); }}
}}
"""

    with open(OUTPUT, "w") as f:
        f.write(header)

    print(f"\nGenerated {os.path.relpath(OUTPUT, SCRIPT_DIR)}")
    print("Run ./compile.sh to build.\n")


if __name__ == "__main__":
    main()
