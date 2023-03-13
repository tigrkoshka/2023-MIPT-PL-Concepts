#pragma once

#include <sstream>      // for ostringstream
#include <string>       // for string, to_string

#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "utils/concepts.hpp"
#include "utils/error.hpp"

namespace karma::errors::executor {

struct Error : errors::Error {
   protected:
    explicit Error(const std::string& message)
        : errors::Error(message) {}
};

struct InternalError : Error {
   private:
    explicit InternalError(const std::string& message)
        : Error("internal executor error: " + message) {}

   public:
    static InternalError UnprocessedCommandFormat(detail::specs::cmd::Format);

    static InternalError UnprocessedCommandForFormat(detail::specs::cmd::Format,
                                                     detail::specs::cmd::Code);
};

struct ExecutionError : Error {
   private:
    explicit ExecutionError(const std::string& message)
        : Error("execution error" + message) {}

   public:
    static ExecutionError ExecPointerOutOfMemory(detail::specs::arch::Address);

    static ExecutionError UnknownCommand(detail::specs::cmd::Code);

    static ExecutionError UnknownSyscallCode(detail::specs::cmd::syscall::Code);

    template <detail::utils::concepts::IntOrFloat T>
    static ExecutionError DivisionByZero(T dividend, T divisor) {
        std::ostringstream ss;
        ss << "a division by zero occurred when dividing " << dividend << " by "
           << divisor;
        return ExecutionError{ss.str()};
    }

    static ExecutionError QuotientOverflow(
        detail::specs::arch::TwoWords dividend,
        detail::specs::arch::TwoWords divisor);

    static ExecutionError BitwiseRHSTooBig(detail::specs::arch::Word,
                                           detail::specs::cmd::Code);

    static ExecutionError DtoiOverflow(detail::specs::arch::Double);

    static ExecutionError InvalidPutCharValue(detail::specs::arch::Word);

    static ExecutionError AddressOutsideOfMemory(
        detail::specs::cmd::args::Address);
};

}  // namespace karma::errors::executor