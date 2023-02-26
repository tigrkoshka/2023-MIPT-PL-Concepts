#pragma once

#include <concepts>  // for totally_ordered
#include <string>    // for string
#include <vector>    // for vector

#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/flags.hpp"

namespace karma {

class Executor {
   public:
    struct Error;
    struct InternalError;
    struct ExecutionError;
    struct ExecFileError;

   private:
    static detail::specs::arch::Double ToDbl(detail::specs::arch::TwoWords);
    static detail::specs::arch::TwoWords ToUll(detail::specs::arch::Double);

    [[nodiscard]] detail::specs::arch::TwoWords GetTwoRegisters(
        detail::specs::cmd::args::Source) const;
    void PutTwoRegisters(detail::specs::arch::TwoWords,
                         detail::specs::cmd::args::Receiver);

    template <std::totally_ordered T>
    void WriteComparisonToFlags(T, T);

    void Jump(detail::specs::flags::Flag, detail::specs::cmd::args::Address);

    void Divide(detail::specs::arch::TwoWords,
                detail::specs::arch::TwoWords,
                detail::specs::cmd::args::Receiver);

    bool Syscall(detail::specs::cmd::args::Register,
                 detail::specs::cmd::syscall::Code);

    void Push(detail::specs::arch::Word);

    void Pop(detail::specs::cmd::args::Receiver,
             detail::specs::cmd::args::Modifier);

    detail::specs::cmd::args::Address Call(detail::specs::cmd::args::Address);

    bool ExecuteRMCommand(detail::specs::cmd::Code,
                          detail::specs::cmd::args::Register,
                          detail::specs::cmd::args::Address);

    bool ExecuteRRCommand(detail::specs::cmd::Code,
                          detail::specs::cmd::args::Receiver,
                          detail::specs::cmd::args::Source,
                          detail::specs::cmd::args::Modifier);

    bool ExecuteRICommand(detail::specs::cmd::Code,
                          detail::specs::cmd::args::Register,
                          detail::specs::cmd::args::Immediate);

    bool ExecuteJCommand(detail::specs::cmd::Code,
                         detail::specs::cmd::args::Address);

    bool ExecuteCommand(detail::specs::cmd::Bin);

    void ExecuteImpl(const std::string& exec_path);

   public:
    void Execute(const std::string& exec_path);

   private:
    std::vector<detail::specs::arch::Word> memory_{
        detail::specs::arch::kMemorySize,
    };

    std::vector<detail::specs::arch::Register> registers_{
        detail::specs::arch::kNRegisters,
    };

    detail::specs::arch::Word flags_{0};
};

}  // namespace karma
