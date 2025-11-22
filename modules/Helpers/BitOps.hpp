#include <cstdint>

using BitMask = std::uint64_t;

namespace helper {

  inline bool get_bit(BitMask x, std::size_t i) {
    return (x >> i) & 1u;
  }

  inline void set_bit(BitMask &x, std::size_t i, bool val) {
    auto mask = BitMask{1} << i;
    if (val) x |= mask;
    else x &= ~mask;
  }

  inline void flip_bit(BitMask &x, std::size_t i) {
    auto mask = BitMask{1} << i;
    x ^= mask;
  }
}
