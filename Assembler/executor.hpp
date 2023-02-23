#pragma once

#include <cmath>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "specs.hpp"

namespace karma {

class Executor {
   private:
    using Word     = uint32_t;
    using Register = uint32_t;
    using Address  = uint32_t;

   private:
    enum Condition : uint32_t {
        EQUAL            = 0b1u,
        NOT_EQUAL        = 0b10u,
        GREATER          = 0b100u,
        LESS             = 0b1000u,
        GREATER_OR_EQUAL = 0b10000u,
        LESS_OR_EQUAL    = 0b100000u,
    };

    struct Errors {
        static std::string UnknownFormat(detail::CommandFormat format);

        static std::string UnknownCommand(detail::CommandFormat format,
                                          detail::CommandCode code);

        static std::string UnknownSyscallCode(detail::SyscallCode code);

        template <typename T>
            requires std::is_same_v<T, Word> || std::is_same_v<T, uint64_t>
        static std::string ImmediateAdditionOverflow(T src_val, T immediate);

        template <typename T>
            requires std::is_same_v<T, uint64_t> || std::is_same_v<T, double>
        static std::string DivisionByZero(T dividend, T divisor);

        static std::string QuotientOverflow(uint64_t dividend,
                                            uint64_t divisor);

        static std::string DtoiOverflow(double value);

        static std::string InvalidPutCharValue(Word value);
    };

   private:
    static double ToDbl(uint64_t ull);
    static uint64_t ToUll(double dbl);

    uint64_t GetTwoRegisters(Register lower) const;
    void PutTwoRegisters(uint64_t value, Register lower);

    template <typename T>
        requires std::totally_ordered<T>
    void WriteComparisonToFlags(T lhs, T rhs);
    void Jump(Condition condition, Address dst);

    void Divide(uint64_t lhs, uint64_t rhs, Register recv);
    bool Syscall(Register reg, detail::SyscallCode code);
    void Push(Word value);
    void Pop(Register recv, Word modifier);
    Word Call(Address callee);

    bool ExecuteRMCommand(detail::CommandCode command_code,
                          Register reg,
                          Address mem);
    bool ExecuteRRCommand(detail::CommandCode command_code,
                          Register recv,
                          Register src,
                          Word immediate);
    bool ExecuteRICommand(detail::CommandCode command_code,
                          Register reg,
                          Word immediate);
    bool ExecuteJCommand(detail::CommandCode command_code, Word immediate);
    bool ExecuteCommand(Word command);

    void ExecuteImpl(const std::string& exec_path);

   public:
    Executor()
        : memory_(kMemorySize),
          registers_(kRegistersNum),
          flags_(0) {}

    void Execute(const std::string& exec_path);

   private:
    static const size_t kWordSize      = 32ull;
    static const Word kMaxWord         = std::numeric_limits<Word>::max();
    static const uint64_t kMaxTwoWords = std::numeric_limits<uint64_t>::max();
    static const uint8_t kMaxChar      = std::numeric_limits<uint8_t>::max();

    static const size_t kMemorySize = 1ull << 20ull;
    std::vector<Word> memory_;

    static const size_t kRegistersNum        = 16;
    static const size_t kStackRegister       = 14;
    static const size_t kInstructionRegister = 15;
    std::vector<Word> registers_;

    static const Word kEqual   = EQUAL + GREATER_OR_EQUAL + LESS_OR_EQUAL;
    static const Word kGreater = NOT_EQUAL + GREATER + GREATER_OR_EQUAL;
    static const Word kLess    = NOT_EQUAL + LESS + LESS_OR_EQUAL;
    Word flags_;
};

}  // namespace karma
