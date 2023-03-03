#include "disassembler_errors.hpp"

#include <iomanip>  // for quoted
#include <sstream>  // for ostringstream
#include <string>   // for string

#include "../specs/commands.hpp"

namespace karma::errors::disassembler {

namespace cmd  = detail::specs::cmd;
namespace args = cmd::args;

////////////////////////////////////////////////////////////////////////////////
///                              Internal errors                             ///
////////////////////////////////////////////////////////////////////////////////

InternalError InternalError::FailedToOpen(const std::string& path) {
    std::ostringstream ss;
    ss << "failed to open " << std::quoted(path);
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

InternalError InternalError::UnknownCommandFormat(cmd::Format format) {
    std::ostringstream ss;
    ss << "unknown command format " << format;
    return InternalError{ss.str()};
}

////////////////////////////////////////////////////////////////////////////////
///                           Disassembling errors                           ///
////////////////////////////////////////////////////////////////////////////////

DisassembleError DisassembleError::UnknownCommand(cmd::Code code) {
    std::ostringstream ss;
    ss << "unknown command code " << code;
    return DisassembleError{ss.str()};
}

}  // namespace karma::errors::disassembler