#pragma once

#include <optional>  // for optional
#include <string>    // for string

#include "compiler/compiler.hpp"
#include "specs/architecture.hpp"
#include "utils/traits.hpp"

namespace karma {

class Compiler::Entrypoint : detail::utils::traits::NonCopyableMovable {
   private:
    using MaybeAddress = std::optional<detail::specs::arch::Address>;
    using MaybePos     = std::optional<std::string>;

   public:
    void Record(detail::specs::arch::Address address,
                const std::string& pos);

    void SetAddress(detail::specs::arch::Address address);

    [[nodiscard]] MaybeAddress TryGetAddress() const;
    [[nodiscard]] MaybePos TryGetPos() const;

    [[nodiscard]] bool Seen() const;

   private:
    MaybeAddress address_;
    MaybePos pos_;
};

}  // namespace karma