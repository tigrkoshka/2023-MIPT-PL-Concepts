#pragma once

#include <concepts>  // for totally_ordered

#include "executor/errors.hpp"
#include "executor/executor.hpp"
#include "executor/executor_base.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/flags.hpp"

namespace karma {

class Executor::CommonExecutor : public ExecutorBase {
   private:
    using ExecutionError = errors::executor::ExecutionError::Builder;

   protected:
    static void CheckBitwiseShiftRHS(detail::specs::arch::Word,
                                     detail::specs::cmd::Code);

    detail::specs::arch::TwoWords GetTwoRegisters(
        detail::specs::cmd::args::Register low);

    void PutTwoRegisters(detail::specs::arch::TwoWords,
                         detail::specs::cmd::args::Receiver);

    void Divide(detail::specs::arch::TwoWords,
                detail::specs::arch::TwoWords,
                detail::specs::cmd::args::Receiver);

    template <std::totally_ordered T>
    void WriteComparisonToFlags(T lhs, T rhs) {
        auto cmp_res = lhs <=> rhs;

        if (cmp_res < 0) {  // NOLINT(hicpp-use-nullptr,modernize-use-nullptr)
            Flags() = detail::specs::flags::kLess;
            return;
        }

        if (cmp_res > 0) {  // NOLINT(hicpp-use-nullptr,modernize-use-nullptr)
            Flags() = detail::specs::flags::kGreater;
            return;
        }

        // condition "cmp_res == 0" is true
        // because lhs and rhs are comparable
        // because of the template type constraint
        Flags() = detail::specs::flags::kEqual;
    }

    void Jump(detail::specs::flags::Flag, detail::specs::cmd::args::Address);

    void Push(detail::specs::arch::Word);

    void Pop(detail::specs::cmd::args::Receiver,
             detail::specs::arch::Word,
             bool internal_usage = false);

    void PrepareCall();
    detail::specs::cmd::args::Address Call(detail::specs::cmd::args::Address);
    void Return();
};

}  // namespace karma
