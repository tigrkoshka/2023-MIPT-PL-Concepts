#pragma once

#include <concepts>   // for totally_ordered
#include <stdexcept>  // for runtime_error
#include <string>     // for string
#include <vector>     // for vector

#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/flags.hpp"

namespace karma {

class Executor {
   private:
    static detail::specs::arch::Double ToDbl(detail::specs::arch::TwoWords);
    static detail::specs::arch::TwoWords ToUll(detail::specs::arch::Double);

    [[nodiscard]] detail::specs::arch::TwoWords GetTwoRegisters(
        detail::specs::cmd::args::Source) const;

    void PutTwoRegisters(detail::specs::arch::TwoWords,
                         detail::specs::cmd::args::Receiver);

    static void CheckBitwiseRHS(detail::specs::arch::types::Word,
                                detail::specs::cmd::Code);

    template <std::totally_ordered T>
    void WriteComparisonToFlags(T, T);

    void Jump(detail::specs::flags::Flag, detail::specs::cmd::args::Address);

    void Divide(detail::specs::arch::types::TwoWords,
                detail::specs::arch::types::TwoWords,
                detail::specs::cmd::args::Receiver);

    bool Syscall(detail::specs::cmd::args::Register,
                 detail::specs::cmd::syscall::Code);

    void Push(detail::specs::arch::types::Word);

    void Pop(detail::specs::cmd::args::Receiver,
             detail::specs::arch::types::Word);

    detail::specs::cmd::args::Address Call(detail::specs::cmd::args::Address);

    bool ExecuteRMCommand(detail::specs::cmd::Code,
                          detail::specs::cmd::args::RMArgs);

    bool ExecuteRRCommand(detail::specs::cmd::Code,
                          detail::specs::cmd::args::RRArgs);

    bool ExecuteRICommand(detail::specs::cmd::Code,
                          detail::specs::cmd::args::RIArgs);

    bool ExecuteJCommand(detail::specs::cmd::Code,
                         detail::specs::cmd::args::JArgs);

    bool ExecuteCommand(detail::specs::cmd::Bin);

    void ExecuteImpl(const std::string& exec_path);

   public:
    void MustExecute(const std::string& exec_path);
    void Execute(const std::string& exec_path);

    Executor()
        : memory_(detail::specs::arch::kMemorySize),
          registers_(detail::specs::arch::kNRegisters) {}

    // Non-copyable
    Executor(const Executor&)           = delete;
    Executor& operator=(const Executor&) = delete;

   private:
    std::vector<detail::specs::arch::types::Word> memory_;

    std::vector<detail::specs::arch::Register> registers_;

    detail::specs::arch::types::Word flags_{0};
};

}  // namespace karma
