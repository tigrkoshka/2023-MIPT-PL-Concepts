#pragma once

#include <unordered_map>

namespace karma {
namespace detail {
namespace utils {

template <typename K, typename V>
using Map = std::unordered_map<K, V>;

template <typename K, typename V>
Map<V, K> RevertMap(const Map<K, V>& m) {
    Map<V, K> reverted;
    reverted.reserve(m.size());

    for (const auto& [k, v] : m) {
        reverted[v] = k;
    }

    return reverted;
}

// GetSignedValue expects bit_size to be no more than 31
// and the signed value to be in the two's complement representation (see:
// https://en.m.wikipedia.org/wiki/Signed_number_representations#Two.27s_complement)
int32_t GetSignedInt(uint32_t ui, uint32_t bit_size) {
    uint32_t sign_bit = 1u << (bit_size - 1);

    if ((ui & sign_bit) > 0) {
        uint32_t modulo = (1u << bit_size) - 1 - ui;
        return -static_cast<int32_t>(modulo);
    }

    return static_cast<int32_t>(ui);
}

}  // namespace utils
}  // namespace detail
}  // namespace karma
