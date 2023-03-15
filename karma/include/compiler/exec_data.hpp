#pragma once

#include <vector>  // for vector

#include "compiler/compiler.hpp"
#include "specs/architecture.hpp"
#include "utils/traits.hpp"

namespace karma {

class Compiler::ExecData : karma::detail::utils::traits::NonCopyableMovable {
   private:
    using Segment = std::vector<karma::detail::specs::arch::Word>;

   public:
    Segment& Code();
    [[nodiscard]] const Segment& Code() const;

    Segment& Constants();
    [[nodiscard]] const Segment& Constants() const;

   private:
    std::vector<karma::detail::specs::arch::Word> code_;
    std::vector<karma::detail::specs::arch::Word> constants_;
};

}  // namespace karma