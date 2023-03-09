#include "types.hpp"

namespace karma::detail::utils::types {

namespace arch = specs::arch;

arch::Double ToDbl(arch::TwoWords ull) {
    return *reinterpret_cast<arch::Double*>(&ull);
}

arch::TwoWords ToUll(arch::Double dbl) {
    return *reinterpret_cast<arch::TwoWords*>(&dbl);
}

std::pair<arch::Word, arch::Word> Split(arch::TwoWords ull) {
    return {
        static_cast<arch::Word>(ull),
        static_cast<arch::Word>(ull >> arch::kWordSize),
    };
}

arch::TwoWords Join(arch::Word low, arch::Word high) {
    return (static_cast<arch::TwoWords>(high) << arch::kWordSize) +
           static_cast<arch::TwoWords>(low);
}

}  // namespace karma::detail::utils::types
