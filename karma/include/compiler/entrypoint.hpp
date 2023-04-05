#pragma once

#include <optional>  // for optional
#include <string>    // for string

#include "compiler/compiler.hpp"
#include "compiler/errors.hpp"
#include "specs/architecture.hpp"
#include "utils/traits.hpp"

namespace karma {

// NOLINTNEXTLINE(bugprone-exception-escape)
class Compiler::Entrypoint : detail::utils::traits::NonCopyableMovable {
   private:
    using InternalError = errors::compiler::InternalError::Builder;
    using CompileError  = errors::compiler::CompileError::Builder;

    using MaybeAddress = std::optional<detail::specs::arch::Address>;
    using MaybePos     = std::optional<std::string>;

   public:
    void Merge(Entrypoint&& other);

    void Record(detail::specs::arch::Address address, const std::string& pos);
    void SetAddress(detail::specs::arch::Address address);

    [[nodiscard]] MaybeAddress TryGetAddress() const;
    [[nodiscard]] MaybePos TryGetPos() const;

   private:
    MaybeAddress address_;
    MaybePos pos_;
};

}  // namespace karma
