#include "utils.hpp"

namespace karma::detail::utils {

int32_t GetSignedInt(uint32_t value, uint32_t bit_size) {
    uint32_t sign_bit = 1u << (bit_size - 1);

    if ((value & sign_bit) > 0) {
        uint32_t modulo = (1u << bit_size) - 1 - value;
        return -static_cast<int32_t>(modulo);
    }

    return static_cast<int32_t>(value);
}

}  // namespace karma::detail::utils
