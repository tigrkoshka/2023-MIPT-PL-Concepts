#include "executor_errors.hpp"

#include <iomanip>  // for quoted
#include <sstream>  // for ostringstream
#include <string>   // for string

#include "../specs/architecture.hpp"
#include "../specs/commands.hpp"
#include "../utils/utils.hpp"

namespace karma::errors::executor {

namespace arch  = detail::specs::arch;
namespace types = arch::types;

namespace cmd     = detail::specs::cmd;
namespace args    = cmd::args;
namespace syscall = cmd::syscall;

namespace utils = detail::utils;

////////////////////////////////////////////////////////////////////////////////
///                              Internal errors                             ///
////////////////////////////////////////////////////////////////////////////////

InternalError InternalError::UnknownCommandFormat(cmd::Format format) {
    std::ostringstream ss;
    ss << "unknown command format " << format;
    return InternalError{ss.str()};
}

InternalError InternalError::UnknownCommandForFormat(cmd::Format format,
                                                     cmd::Code code) {
    std::ostringstream ss;
    ss << "unknown command code " << code << " for command format "
       << std::quoted(cmd::kFormatToString.at(format));
    return InternalError{ss.str()};
}

////////////////////////////////////////////////////////////////////////////////
///                             Execution errors                             ///
////////////////////////////////////////////////////////////////////////////////

ExecutionError ExecutionError::UnknownCommand(cmd::Code code) {
    std::ostringstream ss;
    ss << "unknown command code " << code;
    return ExecutionError{ss.str()};
}

ExecutionError ExecutionError::UnknownSyscallCode(syscall::Code code) {
    std::ostringstream ss;
    ss << "unknown syscall code" << code;
    return ExecutionError{ss.str()};
}

ExecutionError ExecutionError::QuotientOverflow(types::TwoWords dividend,
                                                types::TwoWords divisor) {
    std::ostringstream ss;
    ss << "a quotient overflow occurred when dividing " << dividend << " by "
       << divisor;
    return ExecutionError{ss.str()};
}

ExecutionError ExecutionError::BitwiseRHSTooBig(types::Word rhs,
                                                cmd::Code code) {
    std::ostringstream ss;
    ss << "the right hand side of a bitwise operation (code " << code
       << ") must be less than the bit size of a machine word ("
       << sizeof(types::Word) * utils::kByteSize << "), got: " << rhs;
    return ExecutionError{ss.str()};
}

ExecutionError ExecutionError::DtoiOverflow(types::Double value) {
    std::ostringstream ss;
    ss << "a word overflow occurred when casting " << value << " to integer";
    return ExecutionError{ss.str()};
}

ExecutionError ExecutionError::InvalidPutCharValue(types::Word value) {
    std::ostringstream ss;
    ss << "the value in the register for the PUTCHAR syscall is " << value
       << ", which is an invalid char, because it is greater than 255";
    return ExecutionError{ss.str()};
}

ExecutionError ExecutionError::AddressOutsideOfMemory(args::Address address) {
    std::ostringstream ss;
    ss << "address " << std::hex << address << " is outside of memory (size "
       << arch::kMemorySize << ")";
    return ExecutionError{ss.str()};
}

}  // namespace karma::errors::executor