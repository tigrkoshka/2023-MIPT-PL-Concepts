#include "errors.hpp"

#include <cstddef>  // for size_t
#include <sstream>  // for ostringstream
#include <string>   // for string

#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "utils/type_conversions.hpp"

namespace karma::errors::executor {

using IE = InternalError;
using EE = ExecutionError;

namespace arch = detail::specs::arch;

namespace cmd     = detail::specs::cmd;
namespace args    = cmd::args;
namespace syscall = cmd::syscall;

namespace utils = detail::utils;

////////////////////////////////////////////////////////////////////////////////
///                              Internal errors                             ///
////////////////////////////////////////////////////////////////////////////////

IE IE::UnprocessedCommandFormat(cmd::Format format) {
    std::ostringstream ss;
    ss << "processing for format " << cmd::kFormatToString.at(format)
       << " is not implemented";
    return IE{ss.str()};
}

IE IE::UnprocessedCommandForFormat(cmd::Format format, cmd::Code code) {
    std::ostringstream ss;
    ss << "processing for command " << cmd::kCodeToName.at(code)
       << " of format " << cmd::kFormatToString.at(format)
       << " is not implemented";
    return IE{ss.str()};
}

////////////////////////////////////////////////////////////////////////////////
///                             Execution errors                             ///
////////////////////////////////////////////////////////////////////////////////

EE EE::ExecPointerOutOfMemory(arch::Address address) {
    std::ostringstream ss;
    ss << "execution pointer is outside of memory (size " << arch::kMemorySize
       << "): " << std::hex << address;
    return EE{ss.str()};
}

EE EE::StackPointerOutOfMemory(detail::specs::arch::Address address) {
    std::ostringstream ss;
    ss << "address is outside of memory (size " << arch::kMemorySize
       << "): " << std::hex << address;
    return EE{ss.str()};
}

EE EE::StackOverflow(size_t max_stack_size) {
    std::ostringstream ss;
    ss << "stack overflowed its maximum size (" << max_stack_size << ")";
    return EE{ss.str()};
}

EE EE::InvalidRegister(detail::specs::arch::Register reg) {
    std::ostringstream ss;
    ss << "unknown register " << reg << "there are only " << arch::kNRegisters
       << "registers besides the flags register (from "
       << arch::kRegisterNumToName.at(arch::R0) << "to "
       << arch::kRegisterNumToName.at(arch::R15) << ")";
    return EE{ss.str()};
}

EE EE::RegisterIsBlocked(detail::specs::arch::Register reg) {
    std::ostringstream ss;
    ss << "trying to access a blocked register "
       << arch::kRegisterNumToName.at(reg);
    return EE{ss.str()};
}

EE EE::AddressOutOfMemory(detail::specs::arch::Address address) {
    std::ostringstream ss;
    ss << "trying to access address outside of memory (size "
       << arch::kMemorySize << "): " << std::hex << address;
    return EE{ss.str()};
}

EE EE::CodeSegmentBlocked(detail::specs::arch::Address address) {
    std::ostringstream ss;
    ss << "trying to access address inside a blocked code segment" << std::hex
       << address;
    return EE{ss.str()};
}

EE EE::ConstantsSegmentBlocked(detail::specs::arch::Address address) {
    std::ostringstream ss;
    ss << "trying to access address inside a blocked constants segment"
       << std::hex << address;
    return EE{ss.str()};
}

EE EE::UnknownCommand(cmd::Code code) {
    std::ostringstream ss;
    ss << "unknown command code " << code;
    return EE{ss.str()};
}

EE EE::UnknownSyscallCode(syscall::Code code) {
    std::ostringstream ss;
    ss << "unknown syscall code" << code;
    return EE{ss.str()};
}

EE EE::QuotientOverflow(arch::TwoWords dividend, arch::TwoWords divisor) {
    std::ostringstream ss;
    ss << "a quotient overflow occurred when dividing " << dividend << " by "
       << divisor;
    return EE{ss.str()};
}

EE EE::BitwiseRHSTooBig(arch::Word rhs, cmd::Code code) {
    std::ostringstream ss;
    ss << "the right hand side of a bitwise operation (code " << code
       << ") must be less than the bit size of a machine word ("
       << arch::kWordSize * utils::types::kByteSize << "), got: " << rhs;
    return EE{ss.str()};
}

EE EE::DtoiOverflow(arch::Double value) {
    std::ostringstream ss;
    ss << "a word overflow occurred when casting " << value << " to integer";
    return EE{ss.str()};
}

EE EE::InvalidPutCharValue(arch::Word value) {
    std::ostringstream ss;
    ss << "the value in the register for the PUTCHAR syscall is " << value
       << ", which is an invalid char, because it is greater than 255";
    return EE{ss.str()};
}

}  // namespace karma::errors::executor