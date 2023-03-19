#include "errors.hpp"

#include <iomanip>  // for quoted
#include <sstream>  // for ostringstream
#include <string>   // for string

#include "specs/commands.hpp"
#include "specs/constants.hpp"

namespace karma::errors::disassembler {

using IE = InternalError;
using DE = DisassembleError;

namespace cmd    = detail::specs::cmd;
namespace args   = cmd::args;
namespace consts = detail::specs::consts;

////////////////////////////////////////////////////////////////////////////////
///                              Internal errors                             ///
////////////////////////////////////////////////////////////////////////////////

IE IE::Builder::FailedToOpen(const std::string& path) {
    std::ostringstream ss;
    ss << "failed to open " << std::quoted(path);
    return IE{ss.str()};
}

IE IE::Builder::UnprocessedConstantType(consts::Type type) {
    std::ostringstream ss;
    ss << "processing for constant of type " << consts::kTypeToName.at(type)
       << " is not implemented";
    return IE{ss.str()};
}

IE IE::Builder::CommandNameNotFound(cmd::Code code) {
    std::ostringstream ss;
    ss << "name not found for command with code " << code;
    return IE{ss.str()};
}

IE IE::Builder::RegisterNameNotFound(args::Register reg) {
    std::ostringstream ss;
    ss << "name not found for register # " << reg;
    return IE{ss.str()};
}

IE IE::Builder::UnprocessedCommandFormat(cmd::Format format) {
    std::ostringstream ss;
    ss << "processing for command format " << cmd::kFormatToString.at(format)
       << " is not implemented";
    return IE{ss.str()};
}

////////////////////////////////////////////////////////////////////////////////
///                           Disassembling errors                           ///
////////////////////////////////////////////////////////////////////////////////

DE DE::Builder::UnknownConstantType(consts::Type type) {
    std::ostringstream ss;
    ss << "unknown constant type " << type;
    return DE{ss.str()};
}

DE DE::Builder::ConstantNoValue(size_t start,
                                consts::Type expected,
                                size_t index,
                                size_t constants_size) {
    std::ostringstream ss;
    ss << "expected a " << consts::kTypeToName.at(expected)
       << " constant starting from position " << start
       << ", but reached the end of the constants segment (size "
       << constants_size << ") when trying to read the word #"
       << index - start + 1 << " of the value representation";
    return DE{ss.str()};
}

DE DE::Builder::NoTrailingZeroInString(size_t start) {
    std::ostringstream ss;
    ss << "reached the end of the constants segment, but "
       << "no trailing \'0\' character was found for a string starting from "
       << start;
    return DE{ss.str()};
}

DE DE::Builder::UnknownCommand(cmd::Code code) {
    std::ostringstream ss;
    ss << "unknown command code " << code;
    return DE{ss.str()};
}

}  // namespace karma::errors::disassembler
