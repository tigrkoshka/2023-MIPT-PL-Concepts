#pragma once

#include <cstddef>      // for size_t
#include <sstream>      // for ostringstream
#include <string>       // for string, to_string
#include <type_traits>  // for is_same_v

#include "../specs/architecture.hpp"
#include "../specs/commands.hpp"
#include "error.hpp"

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
    static InternalError UnknownCommandFormat(detail::specs::cmd::Format);

    static InternalError UnknownCommandForFormat(detail::specs::cmd::Format,
                                                 detail::specs::cmd::Code);
};

struct ExecutionError : Error {
   private:
    explicit ExecutionError(const std::string& message)
        : Error("execution error" + message) {}

   public:
    static ExecutionError UnknownCommand(detail::specs::cmd::Code);

    static ExecutionError UnknownSyscallCode(detail::specs::cmd::syscall::Code);

    template <typename T>
        requires std::is_same_v<T, detail::specs::arch::types::TwoWords> ||
                 std::is_same_v<T, detail::specs::arch::types::Double>
    static ExecutionError DivisionByZero(T dividend, T divisor) {
        std::ostringstream ss;
        ss << "a division by zero occurred when dividing " << dividend << " by "
           << divisor;
        return ExecutionError{ss.str()};
    }

    static ExecutionError QuotientOverflow(
        detail::specs::arch::types::TwoWords dividend,
        detail::specs::arch::types::TwoWords divisor);

    static ExecutionError BitwiseRHSTooBig(detail::specs::arch::types::Word,
                                           detail::specs::cmd::Code);

    static ExecutionError DtoiOverflow(detail::specs::arch::types::Double);

    static ExecutionError InvalidPutCharValue(detail::specs::arch::types::Word);

    static ExecutionError AddressOutsideOfMemory(
        detail::specs::cmd::args::Address);
};

}  // namespace karma::errors::executor