#pragma once

#include <concepts>
#include <cstdint>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace karma::detail::util {

// TODO: turn formatting on when clang-format@16 (in llvm@16) is released
// clang-format does not yet properly support concepts definition
// clang-format off
template <typename T>
concept Hashable = requires(T val) {
    { std::hash<T>{}(val) } -> std::convertible_to<std::size_t>;
};
// clang-format on

template <Hashable K, Hashable V>
std::unordered_map<V, K> RevertMap(const std::unordered_map<K, V>& map) {
    std::unordered_map<V, K> reverted;
    reverted.reserve(map.size());

    for (const auto& [k, v] : map) {
        reverted[v] = k;
    }

    return reverted;
}

/**
 * @return
 * The low bit_size bits of value interpreted as a signed integer
 *
 * @param
 * value must be in the two's complement representation
 * (see <a href=https://shorturl.at/gkmtC>Wikipedia</a> for explanation)
 *
 * @param
 * bit_size must be in (0, 8 * sizeof(value)]
 */
template <std::unsigned_integral Uint>
std::make_signed_t<Uint> GetSigned(Uint value, uint32_t bit_size) {
    using Int = std::make_signed_t<Uint>;

    if (bit_size == sizeof value) {
        return *reinterpret_cast<Int*>(&value);
    }

    // clear high bits
    uint32_t shift = sizeof value - bit_size;
    value          = value << shift >> shift;

    Uint sign_bit = 1u << (bit_size - 1);

    if ((value & sign_bit) > 0) {
        Uint modulo = (1u << bit_size) - 1 - value;
        return -static_cast<Int>(modulo);
    }

    return static_cast<Int>(value);
}

}  // namespace karma::detail::util
