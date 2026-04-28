#include <functional>
#include <cstddef>
#include <stdexcept>
#include <ostream>
#include "Base.hpp"

namespace topo {

  struct DragonTruple {
    int group_id;
    int switch_id;
    int endpoint_id;
  };

  // Op overloads allowing engine comparison
  inline bool operator==(const DragonTruple& a, const DragonTruple& b) {
    return a.group_id    == b.group_id &&
       a.switch_id   == b.switch_id &&
       a.endpoint_id == b.endpoint_id;
  }

  inline bool operator!=(const DragonTruple& a, const DragonTruple& b) {
    return !(a == b);
  }

  inline std::ostream& operator<<(std::ostream& os, const DragonTruple& out) {
    return os << "[g=" << out.group_id
              << ", s=" << out.switch_id
              << ", e=" << out.endpoint_id << "]";
  }

  class Dragonfly : public BaseTopo<Dragonfly, DragonTruple> {
    // Classical Dally/Kim style
    private:
      const std::size_t num_endpoints; // Per switch
      const std::size_t num_switches; // Per group
      const std::size_t num_groups;
      const std::size_t num_global_links; // Per switch
    
    public:
      
      using node_type = DragonTruple;
      
      explicit Dragonfly(std::size_t groups,
                         std::size_t switches_per_group,
                         std::size_t endpoints_per_switch)
      : num_endpoints(endpoints_per_switch),
        num_switches(switches_per_group),
        num_groups(groups),
        num_global_links(_get_global_links()) {
        if(num_groups == 0 || num_switches == 0 || num_endpoints == 0) {
          throw std::invalid_argument("Dragonfly requires groups, switches and endpoints to be > 0.");
        }
        if((num_groups - 1) % num_switches != 0) {
          throw std::invalid_argument("Dragonfly must satisfy the following: \n [number of global links per switch] = ([number of groups] - 1) / [number of switches per group] \nWhere the number of global links per switch must be a whole number");
        }
      }

      // Using canonical definition of dragonfly
      std::size_t _get_global_links() const {
        return ((num_groups - 1) / num_switches);
      }

      std::size_t node_count_impl() const {
        return (num_endpoints + 1) * num_switches * num_groups;
      }

      template <typename F>
      void for_each_node_impl(F&& f) const {
        for (std::size_t g = 0; g < num_groups; g++) {
          for (std::size_t s = 0; s < num_switches; s++) {

            {
              DragonTruple sw{
                static_cast<int>(g),
                static_cast<int>(s),
                -1
              };
              f(sw);
            }

            for (std::size_t e = 0; e < num_endpoints; e++) {
              DragonTruple ep{
                static_cast<int>(g),
                static_cast<int>(s),
                static_cast<int>(e)
              };
              f(ep);
            }
          }
        }
      }

      template <typename F>
      void for_each_endpoint_impl(F&& f) const {
        for (std::size_t g = 0; g < num_groups; g++) {
          for (std::size_t s = 0; s < num_switches; s++) {
            for (std::size_t e = 0; e < num_endpoints; e++) {
              DragonTruple ep{
                static_cast<int>(g),
                static_cast<int>(s),
                static_cast<int>(e)
              };
              f(ep);
            }
          }
        }
      }

      template <typename F>
      void for_each_neighbour_impl(const DragonTruple& x, F&& f) const {

        // Currently on endpoint so can only go to switch
        if(x.endpoint_id >= 0) {
          DragonTruple sw { // switch
            x.group_id,
            x.switch_id,
            -1
          };
          f(sw);
          return;
        }

        // Currently on a switch, get all endpoints
        for(std::size_t ep = 0; ep < num_endpoints; ep++) {
          DragonTruple ep_node{ // endpoint node
            x.group_id,
            x.switch_id,
            static_cast<int>(ep)
          };
          f(ep_node);
        }

        // Currently on switch, get other switches
        for(std::size_t s = 0; s < num_switches; s++) {
          if(static_cast<int>(s) == x.switch_id) continue;
          DragonTruple os{ // other switch
            x.group_id,
            static_cast<int>(s),
            -1
          };
          f(os);
        }

        // Currently on switch, get other group switches
        const std::size_t start = x.switch_id * num_global_links;
        for(std::size_t i = 0; i < num_global_links; i++) {
          std::size_t offset = start + i;
          std::size_t other_group = (x.group_id + 1 + offset) % num_groups;

          DragonTruple remote_sw{
            static_cast<int>(other_group),
            x.switch_id,
            -1
          };
          f(remote_sw);
        }
      }

      std::size_t degree_impl(const DragonTruple& x) const {
        if(x.endpoint_id >= 0) return 1;

        return num_endpoints + (num_switches - 1) + num_global_links;
      }

      DragonTruple neighbour_at_impl(const DragonTruple& x, std::size_t i) const {
        if (x.endpoint_id >= 0) {
          if (i != 0) {
            throw std::out_of_range("Endpoint has only one neighbour.");
          }
          return DragonTruple{ x.group_id, x.switch_id, -1 };
        }

        const std::size_t deg = degree_impl(x);
        if (i >= deg) {
          throw std::out_of_range("Neighbour index out of range");
        }

        std::size_t idx = i;

        if (idx < num_endpoints) {
          return DragonTruple{
            x.group_id,
            x.switch_id,
            static_cast<int>(idx)
          };
        }
        idx -= num_endpoints;

        if (idx < num_switches - 1) {
          std::size_t s = idx;
          if (s >= static_cast<std::size_t>(x.switch_id)) {
            s++;
          }
          return DragonTruple{
            x.group_id,
            static_cast<int>(s),
            -1
          };
        }
        idx -= (num_switches - 1);

        if (idx >= num_global_links) {
          throw std::logic_error("Dragonfly::neighbour_at_impl: bad global index");
        }

        const std::size_t start = static_cast<std::size_t>(x.switch_id) * num_global_links;
        const std::size_t other_group = (static_cast<std::size_t>(x.group_id) + 1 + start + idx) % num_groups;

        return DragonTruple{
          static_cast<int>(other_group),
          x.switch_id,
          -1
        };
      }

      std::string node_to_string_impl(const DragonTruple& x) const {
        if (x.endpoint_id >= 0) {
          return "g" + std::to_string(x.group_id) +
                 "-s" + std::to_string(x.switch_id) +
                 "-e" + std::to_string(x.endpoint_id);
        } else {
          return "g" + std::to_string(x.group_id) +
                 "-s" + std::to_string(x.switch_id) +
                 "-SW";
        }
      }

      std::size_t _exit_switch_for_group(std::size_t g_src, std::size_t g_dst) const {
        if(g_src == g_dst) throw std::logic_error("_exit_switch_for_group called for same group.");

        std::size_t j = (g_dst + num_groups - g_src - 1) % num_groups;
        return j / num_global_links;
      }

  };
}

namespace std {
  template<>
  struct hash<topo::DragonTruple> {
    size_t operator()(const topo::DragonTruple& x) const noexcept {
      size_t h = 0;
      auto mix = [](size_t h, size_t v) {
        h ^= v + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        return h;
      };
      h = mix(h, std::hash<int>{}(x.group_id));
      h = mix(h, std::hash<int>{}(x.switch_id));
      h = mix(h, std::hash<int>{}(x.endpoint_id));
      return h;
    }
  };
}
