#include "entrypoint.hpp"

#include <string>   // for string
#include <utility>  // for move

#include "compiler/compiler.hpp"
#include "compiler/errors.hpp"
#include "specs/architecture.hpp"

namespace karma {

namespace arch = detail::specs::arch;

void Compiler::Entrypoint::Merge(Entrypoint&& other) {
    if (address_ && other.address_) {
        if (!pos_ || !other.pos_) {
            throw InternalError::EntrypointHasAddressButNotPos();
        }

        throw CompileError::SecondEntrypoint(*other.pos_, *pos_);
    }

    if (!other.address_) {
        return;
    }

    address_ = other.address_;
    pos_     = std::move(other.pos_);
}

void Compiler::Entrypoint::Record(arch::Address address,
                                  const std::string& pos) {
    if (pos_) {
        throw CompileError::SecondEntrypoint(pos, *pos_);
    }

    address_ = address;
    pos_     = pos;
}

void Compiler::Entrypoint::SetAddress(arch::Address address) {
    if (!pos_) {
        throw InternalError::EntrypointSetAddressNoPos();
    }

    address_ = address;
}

Compiler::Entrypoint::MaybeAddress Compiler::Entrypoint::TryGetAddress() const {
    return address_;
}

Compiler::Entrypoint::MaybePos Compiler::Entrypoint::TryGetPos() const {
    return pos_;
}

}  // namespace karma
