#pragma once

#include <concepts>
#include <string>
#include <vector>

#include "specs.hpp"

namespace karma {

class Executor {
   private:
    using Word          = detail::specs::Word;
    using TwoWords      = detail::specs::TwoWords;
    using Double        = detail::specs::Double;
    using CommandCode   = detail::specs::CommandCode;
    using CommandFormat = detail::specs::CommandFormat;
    using SyscallCode   = detail::specs::SyscallCode;
    using Flag          = detail::specs::Flag;
    using CommandBin    = detail::specs::CommandBin;
    using Register      = detail::specs::Register;
    using Receiver      = detail::specs::Receiver;
    using Source        = detail::specs::Source;
    using Address       = detail::specs::Address;
    using Modifier      = detail::specs::Modifier;
    using Immediate     = detail::specs::Immediate;

   public:
    struct Error;
    struct InternalError;
    struct ExecutionError;
    struct ExecFileError;

   private:
    static Double ToDbl(TwoWords);
    static TwoWords ToUll(Double);

    [[nodiscard]] TwoWords GetTwoRegisters(Source) const;
    void PutTwoRegisters(TwoWords, Receiver);

    template <std::totally_ordered T>
    void WriteComparisonToFlags(T, T);
    void Jump(Flag, Address);

    void Divide(TwoWords, TwoWords, Receiver);
    bool Syscall(Register, SyscallCode);
    void Push(Word);
    void Pop(Receiver, Modifier);
    Address Call(Address);

    bool ExecuteRMCommand(CommandCode, Register, Address);
    bool ExecuteRRCommand(CommandCode, Receiver, Source, Modifier);
    bool ExecuteRICommand(CommandCode, Register, Immediate);
    bool ExecuteJCommand(CommandCode, Address);
    bool ExecuteCommand(CommandBin);

    void ExecuteImpl(const std::string& exec_path);

   public:
    void Execute(const std::string& exec_path);

   private:
    std::vector<Word> memory_{detail::specs::kMemorySize};
    std::vector<Word> registers_{detail::specs::kNRegisters};
    Word flags_{0};
};

}  // namespace karma
