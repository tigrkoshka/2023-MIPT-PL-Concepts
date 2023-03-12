#pragma once

#include <unordered_map>  // for unordered_map

#include "concepts.hpp"

namespace karma::detail::utils::map {

template <concepts::Hashable K, concepts::Hashable V>
std::unordered_map<V, K> Revert(const std::unordered_map<K, V>& map) {
    std::unordered_map<V, K> reverted;
    reverted.reserve(map.size());

    for (const auto& [k, v] : map) {
        reverted[v] = k;
    }

    return reverted;
}

}  // namespace karma::detail::utils::map
