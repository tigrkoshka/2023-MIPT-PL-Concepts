#pragma once

#include <concepts>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "specs.hpp"

namespace karma {

class Executor {
   private:
    using CommandCode   = detail::specs::CommandCode;
    using CommandFormat = detail::specs::CommandFormat;
    using SyscallCode   = detail::specs::SyscallCode;
    using Flag          = detail::specs::Flag;
    using Word          = detail::specs::Word;

    // TODO: maybe rethink (detail::specs has a enum)
    using Register = uint32_t;
    using Address  = detail::specs::Address;

   private:
    struct Error;
    struct InternalError;
    struct ExecutionError;
    struct ExecFileError;

   private:
    static double ToDbl(uint64_t ull);
    static uint64_t ToUll(double dbl);

    [[nodiscard]] uint64_t GetTwoRegisters(Register lower) const;
    void PutTwoRegisters(uint64_t value, Register lower);

    template <std::totally_ordered T>
    void WriteComparisonToFlags(T lhs, T rhs);
    void Jump(Flag flag, Address dst);

    void Divide(uint64_t lhs, uint64_t rhs, Register recv);
    bool Syscall(Register reg, SyscallCode code);
    void Push(Word value);
    void Pop(Register recv, Word modifier);
    Word Call(Address callee);

    bool ExecuteRMCommand(CommandCode command_code,
                          Register reg,
                          Address address);
    bool ExecuteRRCommand(CommandCode command_code,
                          Register recv,
                          Register src,
                          int32_t modifier);
    bool ExecuteRICommand(CommandCode command_code,
                          Register reg,
                          int32_t immediate);
    bool ExecuteJCommand(CommandCode command_code, Word immediate);
    bool ExecuteCommand(Word command);

    void ExecuteImpl(const std::string& exec_path);

   public:
    void Execute(const std::string& exec_path);

   private:
    static const Word kMaxWord         = std::numeric_limits<Word>::max();
    static const uint64_t kMaxTwoWords = std::numeric_limits<uint64_t>::max();
    static const uint8_t kMaxChar      = std::numeric_limits<char8_t>::max();

    std::vector<Word> memory_{detail::specs::kMemorySize};
    std::vector<Word> registers_{detail::specs::kNRegisters};
    Word flags_{0};
};

}  // namespace karma
