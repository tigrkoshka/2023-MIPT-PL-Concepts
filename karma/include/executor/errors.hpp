#pragma once

#include <cstddef>  // for size_t
#include <sstream>  // for ostringstream
#include <string>   // for string, to_string

#include "executor/executor.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "utils/concepts.hpp"
#include "utils/traits.hpp"

namespace karma::errors::executor {

struct InternalError::Builder : detail::utils::traits::Static {
    static InternalError UnprocessedCommandFormat(detail::specs::cmd::Format);

    static InternalError UnprocessedCommandForFormat(detail::specs::cmd::Format,
                                                     detail::specs::cmd::Code);
};

struct ExecutionError::Builder : detail::utils::traits::Static {
    static ExecutionError ExecPointerOutOfMemory(detail::specs::arch::Address);
    static ExecutionError StackPointerOutOfMemory(detail::specs::arch::Address);
    static ExecutionError StackOverflow(size_t max_stack_size);

    static ExecutionError InvalidRegister(detail::specs::arch::Register);
    static ExecutionError RegisterIsBlocked(detail::specs::arch::Register);

    static ExecutionError AddressOutOfMemory(detail::specs::arch::Address);
    static ExecutionError CodeSegmentBlocked(detail::specs::arch::Address);
    static ExecutionError ConstantsSegmentBlocked(detail::specs::arch::Address);

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
};

}  // namespace karma::errors::executor