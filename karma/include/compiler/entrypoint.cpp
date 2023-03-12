#include "entrypoint.hpp"

#include <string>  // for string

#include "specs/architecture.hpp"

namespace karma::compiler::detail {

namespace arch = karma::detail::specs::arch;

void Entrypoint::Record(arch::Address address, const std::string& pos) {
    address_ = address;
    pos_     = pos;
}

void Entrypoint::SetAddress(arch::Address address) {
    address_ = address;
}

Entrypoint::MaybeAddress Entrypoint::TryGetAddress() const {
    return address_;
}

Entrypoint::MaybePos Entrypoint::TryGetPos() const {
    return pos_;
}

bool Entrypoint::Seen() const {
    return address_ != std::nullopt;
}

}  // namespace karma::compiler::detail
