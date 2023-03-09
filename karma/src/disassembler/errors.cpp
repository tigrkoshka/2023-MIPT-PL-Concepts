#include "errors.hpp"

#include <iomanip>  // for quoted
#include <sstream>  // for ostringstream
#include <string>   // for string

#include "../specs/commands.hpp"
#include "../specs/constants.hpp"

namespace karma::errors::disassembler {

namespace cmd  = detail::specs::cmd;
namespace args = cmd::args;

namespace consts = detail::specs::consts;

////////////////////////////////////////////////////////////////////////////////
///                              Internal errors                             ///
////////////////////////////////////////////////////////////////////////////////

InternalError InternalError::FailedToOpen(const std::string& path) {
    std::ostringstream ss;
    ss << "failed to open " << std::quoted(path);
    return InternalError{ss.str()};
}

InternalError InternalError::UnprocessedConstantType(consts::Type type) {
    std::ostringstream ss;
    ss << "processing for constant of type " << consts::kTypeToName.at(type)
       << " is not implemented";
    return InternalError{ss.str()};
}

InternalError InternalError::CommandNameNotFound(cmd::Code code) {
    std::ostringstream ss;
    ss << "name not found for command with code " << code;
    return InternalError{ss.str()};
}

InternalError InternalError::RegisterNameNotFound(args::Register reg) {
    std::ostringstream ss;
    ss << "name not found for register # " << reg;
    return InternalError{ss.str()};
}

InternalError InternalError::UnprocessedCommandFormat(cmd::Format format) {
    std::ostringstream ss;
    ss << "processing for command format " << cmd::kFormatToString.at(format)
       << " is not implemented";
    return InternalError{ss.str()};
}

////////////////////////////////////////////////////////////////////////////////
///                           Disassembling errors                           ///
////////////////////////////////////////////////////////////////////////////////

DisassembleError DisassembleError::UnknownConstantType(consts::Type type) {
    std::ostringstream ss;
    ss << "unknown constant type " << type;
    return DisassembleError{ss.str()};
}

DisassembleError DisassembleError::ConstantNoValue(size_t start,
                                                   consts::Type expected,
                                                   size_t index,
                                                   size_t constants_size) {
    std::ostringstream ss;
    ss << "expected a " << consts::kTypeToName.at(expected)
       << " constant starting from position " << start
       << ", but reached the end of the constants segment (size "
       << constants_size << ") when trying to read the word #"
       << index - start + 1 << " of the value representation";
    return DisassembleError{ss.str()};
}

DisassembleError DisassembleError::NoTrailingZeroInString(size_t start) {
    std::ostringstream ss;
    ss << "reached the end of the constants segment, but "
       << "no trailing \'0\' character was found for a string starting from "
       << start;
    return DisassembleError{ss.str()};
}

DisassembleError DisassembleError::UnknownCommand(cmd::Code code) {
    std::ostringstream ss;
    ss << "unknown command code " << code;
    return DisassembleError{ss.str()};
}

}  // namespace karma::errors::disassembler