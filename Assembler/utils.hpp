#pragma once

#include <cstdint>
#include <unordered_map>

namespace karma::detail::utils {

template <typename K, typename V>
using Map = std::unordered_map<K, V>;

template <typename K, typename V>
Map<V, K> RevertMap(const Map<K, V>& map) {
    Map<V, K> reverted;
    reverted.reserve(map.size());

    for (const auto& [k, v] : map) {
        reverted[v] = k;
    }

    return reverted;
}

// GetSignedValue expects bit_size to be no more than 31
// and the signed value to be in the two's complement representation (see:
// https://en.m.wikipedia.org/wiki/Signed_number_representations#Two.27s_complement)
int32_t GetSignedInt(uint32_t value, uint32_t bit_size);

}  // namespace karma::detail::utils
