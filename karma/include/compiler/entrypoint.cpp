#include "entrypoint.hpp"

#include <string>  // for string

#include "compiler/compiler.hpp"
#include "specs/architecture.hpp"

namespace karma {

namespace arch = karma::detail::specs::arch;

void Compiler::Entrypoint::Record(arch::Address address,
                                  const std::string& pos) {
    address_ = address;
    pos_     = pos;
}

void Compiler::Entrypoint::SetAddress(arch::Address address) {
    address_ = address;
}

Compiler::Entrypoint::MaybeAddress Compiler::Entrypoint::TryGetAddress() const {
    return address_;
}

Compiler::Entrypoint::MaybePos Compiler::Entrypoint::TryGetPos() const {
    return pos_;
}

bool Compiler::Entrypoint::Seen() const {
    return address_ != std::nullopt;
}

}  // namespace karma
