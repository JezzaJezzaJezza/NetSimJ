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
      const std::size_t num_endpoints; // Per router
      const std::size_t num_switches; // Per group
      const std::size_t num_groups;
      const std::size_t num_global_links; // Per router
    
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
        return num_endpoints * num_switches * num_groups;
      }

      template <typename F>
      void for_each_node_impl(F&& f) const {
        for(std::size_t _group = 0; _group < num_groups; _group++) {
          for(std::size_t _switch = 0; _switch < num_switches; _switch++) {
            for(std::size_t _endpoint = 0; _endpoint < num_endpoints; _endpoint++) {
              DragonTruple x{
                static_cast<int>(_group),
                static_cast<int>(_switch),
                static_cast<int>(_endpoint),
              };
              f(x);
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
        if(x.endpoint_id >= 0) {
          if(i != 0 ) {
            throw std::out_of_range("Endpoint has only one neighbour.");
          }

          return DragonTruple{
            x.group_id,
            x.switch_id,
            -1
          };
        }

        const std::size_t deg = degree_impl(x);
        if(i >= deg) throw std::out_of_range("Neighbour index out of range");

        std::size_t idx = i;

        if(idx < num_endpoints) {
          return DragonTruple{
            x.group_id,
            x.switch_id,
            static_cast<int>(idx)
          };
        }
        idx -= num_switches;

        if(idx < num_switches - 1) {
          std::size_t s = idx;

          if(s >= static_cast<std::size_t>(x.switch_id)) s++;

          return DragonTruple{
            x.group_id,
            static_cast<int>(s),
            -1
          };
        }
        idx -= (num_switches - 1);
  
        if(idx >= num_global_links) throw std::logic_error("Bug in neighbour_at_impl - global part.");

        const std::size_t start = x.switch_id * num_global_links;
        const std::size_t other_group = (x.group_id + 1 + (start + idx)) % num_groups;
        return DragonTruple{
          static_cast<int>(other_group),
          x.switch_id,
          -1
        };
      }

      std::size_t dim_count() const {
        return 3;
      }

      // TODO: Make these passed by ref -> Currently not as need to match function signature of Hypercube
      bool dim_aligned(DragonTruple a, DragonTruple b, std::size_t dim) const {
        switch(dim) {
          case 0:
            return a.group_id == b.group_id;

          case 1:
            return a.group_id == b.group_id &&
                   a.switch_id == b.switch_id;

          case 2:
            return a.group_id == b.group_id &&
                   a.switch_id == b.switch_id &&
                   a.endpoint_id == b.endpoint_id;

          default:
            throw std::out_of_range("Invalid dimension in dim_aligned (for DOR).");
            
        }
      }

      std::size_t _exit_switch_for_group(std::size_t g_src, std::size_t g_dst) const {
        if(g_src == g_dst) throw std::logic_error("_exit_switch_for_group called for same group.");

        std::size_t j = (g_dst + num_groups - g_src - 1) % num_groups;
        return j / num_global_links;
      }

      // TODO: Also look to make this completely reference based
      DragonTruple move_to(DragonTruple from, DragonTruple to, std::size_t dim) const {
        switch(dim) {

          // Group dim
          case 0: {
              if(from.group_id == to.group_id) return from;

              // Endpoint -> switch
              if(from.endpoint_id >= 0) {
                return DragonTruple{
                  from.group_id,
                  from.switch_id,
                  -1
                };
              }

              // Check to see if moving groups possible
              const std::size_t exit_sw = _exit_switch_for_group(from.group_id, to.group_id);
              if(from.switch_id != exit_sw) {
                
                return DragonTruple{
                  from.group_id,
                  static_cast<int>(exit_sw),
                  -1
                };
                  
              } else {
                
                return DragonTruple{
                  to.group_id,
                  from.switch_id,
                  -1
                };
              }
            }

          // Switch dim
          case 1: {
              if(from.endpoint_id >= 0) {
                return DragonTruple{
                  from.group_id,
                  from.switch_id,
                  -1
                };
              }

              if(from.switch_id == to.switch_id) {
                return from;
              }

              return DragonTruple{
                from.group_id,
                to.switch_id,
                -1
              };
            }

          // Endpoint dim
          case 2: {
              if(from.group_id == to.group_id &&
                 from.switch_id == to.switch_id &&
                 from.endpoint_id == to.endpoint_id) return from;

              if(from.endpoint_id == -1) {
                return DragonTruple{
                  from.group_id,
                  from.switch_id,
                  to.endpoint_id
                };
              }

              return DragonTruple{
                from.group_id,
                from.switch_id,
                -1
              };
            }

          default:
              throw std::out_of_range("Invalid dimension in move_to.");
            
        }
      }
  };
}

