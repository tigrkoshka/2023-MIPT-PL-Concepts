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

}  // namespace karma::detail::util
