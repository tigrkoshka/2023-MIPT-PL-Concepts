#include "errors.hpp"

#include <iomanip>  // for quoted
#include <sstream>  // for ostringstream
#include <string>   // for string

#include "../specs/architecture.hpp"
#include "../specs/commands.hpp"
#include "../utils/types.hpp"

namespace karma::errors::executor {

namespace arch = detail::specs::arch;

namespace cmd     = detail::specs::cmd;
namespace args    = cmd::args;
namespace syscall = cmd::syscall;

namespace utils = detail::utils;

////////////////////////////////////////////////////////////////////////////////
///                              Internal errors                             ///
////////////////////////////////////////////////////////////////////////////////

InternalError InternalError::UnprocessedCommandFormat(cmd::Format format) {
    std::ostringstream ss;
    ss << "processing for format " << cmd::kFormatToString.at(format)
       << " is not implemented";
    return InternalError{ss.str()};
}

InternalError InternalError::UnprocessedCommandForFormat(cmd::Format format,
                                                         cmd::Code code) {
    std::ostringstream ss;
    ss << "processing for command " << cmd::kCodeToName.at(code)
       << " of format " << cmd::kFormatToString.at(format)
       << " is not implemented";
    return InternalError{ss.str()};
}

////////////////////////////////////////////////////////////////////////////////
///                             Execution errors                             ///
////////////////////////////////////////////////////////////////////////////////

ExecutionError ExecutionError::ExecPointerOutOfMemory(arch::Address address) {
    std::ostringstream ss;
    ss << "execution pointer is outside of memory: " << std::hex << address;
    return ExecutionError{ss.str()};
}

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

ExecutionError ExecutionError::QuotientOverflow(arch::TwoWords dividend,
                                                arch::TwoWords divisor) {
    std::ostringstream ss;
    ss << "a quotient overflow occurred when dividing " << dividend << " by "
       << divisor;
    return ExecutionError{ss.str()};
}

ExecutionError ExecutionError::BitwiseRHSTooBig(arch::Word rhs,
                                                cmd::Code code) {
    std::ostringstream ss;
    ss << "the right hand side of a bitwise operation (code " << code
       << ") must be less than the bit size of a machine word ("
       << arch::kWordSize * utils::types::kByteSize << "), got: " << rhs;
    return ExecutionError{ss.str()};
}

ExecutionError ExecutionError::DtoiOverflow(arch::Double value) {
    std::ostringstream ss;
    ss << "a word overflow occurred when casting " << value << " to integer";
    return ExecutionError{ss.str()};
}

ExecutionError ExecutionError::InvalidPutCharValue(arch::Word value) {
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