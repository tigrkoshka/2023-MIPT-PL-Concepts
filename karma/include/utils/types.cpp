#include "types.hpp"

namespace karma::detail::utils::types {

std::pair<uint32_t, uint32_t> Split(uint64_t ull) {
    return {
        static_cast<uint32_t>(ull),
        static_cast<uint32_t>(ull >> sizeof(uint32_t) * kByteSize),
    };
}

uint64_t Join(uint32_t low, uint32_t high) {
    return (static_cast<uint64_t>(high) << sizeof(uint32_t) * kByteSize) +
           static_cast<uint64_t>(low);
}

}  // namespace karma::detail::utils::types
