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
   public:
    struct Error;
    struct InternalError;
    struct ExecutionError;

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
    void MustExecute(const std::string& exec_path);
    void Execute(const std::string& exec_path);

    Executor()
        : memory_(detail::specs::arch::kMemorySize),
          registers_(detail::specs::arch::kNRegisters) {}

   private:
    std::vector<detail::specs::arch::Word> memory_;

    std::vector<detail::specs::arch::Register> registers_;

    detail::specs::arch::Word flags_{0};
};

struct Executor::Error : std::runtime_error {
   protected:
    explicit Error(const std::string& message)
        : std::runtime_error(message) {}

   public:
    Error(const Error&)            = default;
    Error& operator=(const Error&) = default;
    Error(Error&&)                 = default;
    Error& operator=(Error&&)      = default;
    ~Error() override              = default;
};

struct Executor::InternalError : Error {
   private:
    friend void Executor::Execute(const std::string& exec_path);

   private:
    explicit InternalError(const std::string& message)
        : Error("internal executor error: " + message) {}

   public:
    InternalError(const InternalError&)            = default;
    InternalError& operator=(const InternalError&) = default;
    InternalError(InternalError&&)                 = default;
    InternalError& operator=(InternalError&&)      = default;
    ~InternalError() override                      = default;

   public:
    static InternalError UnknownCommandFormat(detail::specs::cmd::Format);

    static InternalError UnknownCommandForFormat(detail::specs::cmd::Format,
                                                 detail::specs::cmd::Code);
};

struct Executor::ExecutionError : Error {
   private:
    explicit ExecutionError(const std::string& message)
        : Error("execution error" + message) {}

   public:
    ExecutionError(const ExecutionError&)            = default;
    ExecutionError& operator=(const ExecutionError&) = default;
    ExecutionError(ExecutionError&&)                 = default;
    ExecutionError& operator=(ExecutionError&&)      = default;
    ~ExecutionError() override                       = default;

   public:
    static ExecutionError UnknownCommand(detail::specs::cmd::Code);

    static ExecutionError UnknownSyscallCode(detail::specs::cmd::syscall::Code);

    template <typename T>
        requires std::is_same_v<T, detail::specs::arch::types::TwoWords> ||
                 std::is_same_v<T, detail::specs::arch::types::Double>
    static ExecutionError DivisionByZero(T dividend, T divisor);

    static ExecutionError QuotientOverflow(
        detail::specs::arch::types::TwoWords dividend,
        detail::specs::arch::types::TwoWords divisor);

    static ExecutionError DtoiOverflow(detail::specs::arch::types::Double);

    static ExecutionError InvalidPutCharValue(
        detail::specs::cmd::args::Immediate);

    static ExecutionError AddressOutsideOfMemory(
        detail::specs::cmd::args::Address);
};

}  // namespace karma
