#pragma once

#include <concepts>       // for convertible_to
#include <functional>     // for hash
#include <unordered_map>  // for unordered_map

namespace karma::detail::utils::map {

// TODO: turn formatting on when clang-format@16 (in llvm@16) is released
// clang-format does not yet properly support concepts definition
// clang-format off
template <typename T>
concept Hashable = requires(T val) {
    { std::hash<T>{}(val) } -> std::convertible_to<std::size_t>;
};
// clang-format on

template <Hashable K, Hashable V>
std::unordered_map<V, K> Revert(const std::unordered_map<K, V>& map) {
    std::unordered_map<V, K> reverted;
    reverted.reserve(map.size());

    for (const auto& [k, v] : map) {
        reverted[v] = k;
    }

    return reverted;
}

}  // namespace karma::detail::utils::map
