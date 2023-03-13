#pragma once

#include <concepts>  // for totally_ordered

#include "command_executor.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/flags.hpp"

namespace karma::executor::detail {

class CommonExecutor : public CommandExecutor {
   protected:
    karma::detail::specs::arch::TwoWords GetTwoRegisters(
        karma::detail::specs::cmd::args::Register low);

    void PutTwoRegisters(karma::detail::specs::arch::TwoWords,
                         karma::detail::specs::cmd::args::Receiver);

    void CheckBitwiseRHS(karma::detail::specs::arch::Word,
                         karma::detail::specs::cmd::Code);

    template <std::totally_ordered T>
    void WriteComparisonToFlags(T, T);

    void Divide(karma::detail::specs::arch::TwoWords,
                karma::detail::specs::arch::TwoWords,
                karma::detail::specs::cmd::args::Receiver);

    void Push(karma::detail::specs::arch::Word);

    void Pop(karma::detail::specs::cmd::args::Receiver,
             karma::detail::specs::arch::Word);

    void Jump(karma::detail::specs::flags::Flag,
              karma::detail::specs::cmd::args::Address);

    karma::detail::specs::cmd::args::Address Call(
        karma::detail::specs::cmd::args::Address);
    void Return();
};

}  // namespace karma::executor::detail