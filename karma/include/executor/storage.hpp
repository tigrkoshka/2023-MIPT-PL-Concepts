#pragma once

#include <utility>  // for pair
#include <vector>   // for vector

#include "exec/exec.hpp"
#include "specs/architecture.hpp"
#include "utils/traits.hpp"

namespace karma::executor::detail {

class Storage : karma::detail::utils::traits::NonCopyableMovable {
   public:
    Storage()
        : memory_(karma::detail::specs::arch::kMemorySize),
          registers_(karma::detail::specs::arch::kNRegisters) {}

    void PrepareForExecution(const karma::detail::exec::Data& exec_data);

    karma::detail::specs::arch::Word& Reg(karma::detail::specs::arch::Register);
    karma::detail::specs::arch::Word& Mem(karma::detail::specs::arch::Address);
    karma::detail::specs::arch::Word& Flags();

   private:
    std::vector<karma::detail::specs::arch::Word> memory_;
    std::vector<karma::detail::specs::arch::Word> registers_;
    karma::detail::specs::arch::Word flags_{0};
};

}  // namespace karma::executor::detail