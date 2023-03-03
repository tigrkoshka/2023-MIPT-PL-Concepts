#include "compiler_errors.hpp"

#include <cstddef>  // for size_t
#include <iomanip>  // for quoted
#include <sstream>  // for ostringstream
#include <string>   // for string, to_string

#include "../specs/commands.hpp"
#include "../specs/syntax.hpp"

namespace karma::errors::compiler {

namespace cmd = detail::specs::cmd;

namespace syntax = detail::specs::syntax;

////////////////////////////////////////////////////////////////////////////////
///                              Internal errors                             ///
////////////////////////////////////////////////////////////////////////////////

InternalError InternalError::FailedToOpen(const std::string& path) {
    std::ostringstream ss;
    ss << "failed to open " << std::quoted(path);
    return InternalError{ss.str()};
}

InternalError InternalError::FormatNotFound(cmd::Code command_code,
                                            size_t line) {
    std::ostringstream ss;
    ss << "did not find format for command with code " << command_code;
    return InternalError{ss.str(), line};
}

InternalError InternalError::UnknownCommandFormat(cmd::Format format,
                                                  size_t line) {
    std::ostringstream ss;
    ss << "unknown command format " << format;
    return InternalError{ss.str(), line};
}

InternalError InternalError::EmptyWord(size_t line) {
    return InternalError{"the current word is empty", line};
}

////////////////////////////////////////////////////////////////////////////////
///                            Compilation errors                            ///
////////////////////////////////////////////////////////////////////////////////

///----------------------------------Command---------------------------------///

CompileError CompileError::UnknownCommand(const std::string& command,
                                          size_t line) {
    std::ostringstream ss;
    ss << "unknown command " << std::quoted(command);
    return CompileError{ss.str(), line};
}

///--------------------------------Entrypoint--------------------------------///

CompileError CompileError::NoEntrypoint() {
    return CompileError("did not encounter an entrypoint");
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
CompileError CompileError::SecondEntrypoint(size_t line,
                                            size_t entrypoint_line) {
    std::ostringstream ss;
    ss << "encountered second entrypoint (previous one on line "
       << entrypoint_line << ")";
    return CompileError{ss.str(), line};
}

CompileError CompileError::EntrypointWithoutAddress(size_t line) {
    return CompileError{"entrypoint address not specified", line};
}

///----------------------------------Labels----------------------------------///

CompileError CompileError::EmptyLabel(size_t line) {
    return CompileError("label name must not be empty", line);
}

CompileError CompileError::LabelBeforeEntrypoint(
    size_t end_line,
    const std::string& latest_label,
    size_t latest_label_line) {
    std::ostringstream ss;
    ss << "label " << std::quoted(latest_label) << " is placed before the "
       << std::quoted(syntax::kEntrypointDirective) << " directive (at line "
       << end_line << "), but a label can only appear before a command";
    return CompileError{ss.str(), latest_label_line};
}

CompileError CompileError::ConsecutiveLabels(const std::string& label,
                                             size_t line,
                                             const std::string& latest_label,
                                             size_t latest_label_line) {
    std::ostringstream ss;
    ss << "label " << std::quoted(label)
       << " is not separated from the previous one ("
       << std::quoted(latest_label) << " in line " << latest_label_line
       << ") by at least one command";
    return CompileError{ss.str(), line};
}

CompileError CompileError::FileEndsWithLabel(const std::string& label,
                                             size_t line) {
    std::ostringstream ss;
    ss << "label " << std::quoted(label)
       << " is the last non-comment word in file";
    return CompileError{ss.str(), line};
}

CompileError CompileError::LabelStartsWithDigit(const std::string& label,
                                                size_t line) {
    std::ostringstream ss;
    ss << "label " << std::quoted(label) << " starts with a digit";
    return CompileError{ss.str(), line};
}

CompileError CompileError::InvalidLabelCharacter(char invalid,
                                                 const std::string& label,
                                                 size_t line) {
    std::ostringstream ss;
    ss << "label " << std::quoted(label) << " contains an invalid character: \'"
       << invalid << "\' (only latin letters and digits are allowed)";
    return CompileError{ss.str(), line};
}

CompileError CompileError::UndefinedLabel(const std::string& label,
                                          size_t line) {
    std::ostringstream ss;
    ss << "label " << std::quoted(label) << " is not defined";
    return CompileError{ss.str(), line};
}

///---------------------------------Register---------------------------------///

CompileError CompileError::UnknownRegister(const std::string& reg,
                                           size_t line) {
    std::ostringstream ss;
    ss << "unknown register " << std::quoted(reg);
    return CompileError{ss.str(), line};
}

///----------------------------------Address---------------------------------///

CompileError CompileError::AddressNegative(const std::string& address,
                                           size_t line) {
    std::ostringstream ss;
    ss << "the address operand " << std::quoted(address, ')')
       << " must not be negative";
    return CompileError{ss.str(), line};
}

CompileError CompileError::AddressOutOfMemory(const std::string& address,
                                              size_t line) {
    std::ostringstream ss;
    ss << "the address operand " << std::quoted(address, ')')
       << " exceeds the memory size";
    return CompileError{ss.str(), line};
}

///---------------------------------Immediate--------------------------------///

CompileError CompileError::ImmediateNotANumber(const std::string& immediate,
                                               size_t line) {
    std::ostringstream ss;
    ss << "the immediate operand is not a number: " << std::quoted(immediate);
    return CompileError{ss.str(), line};
}

CompileError CompileError::ImmediateLessThanMin(int32_t min,
                                                const std::string& immediate,
                                                size_t line) {
    std::ostringstream ss;
    ss << "the immediate operand is less than the allowed minimum (" << min
       << "): " << immediate;
    return CompileError{ss.str(), line};
}

CompileError CompileError::ImmediateMoreThanMax(int32_t max,
                                                const std::string& immediate,
                                                size_t line) {
    std::ostringstream ss;
    ss << "the immediate operand is less than the allowed maximum (" << max
       << "): " << immediate;
    return CompileError{ss.str(), line};
}

CompileError CompileError::ImmediateOutOfRange(const std::string& immediate,
                                               size_t line) {
    std::ostringstream ss;
    ss << "the immediate operand is out of range " << immediate;
    return CompileError{ss.str(), line};
}

///------------------------------------RM------------------------------------///

CompileError CompileError::RMCommandNoRegister(size_t line) {
    return CompileError{
        "register not specified for RM format command",
        line,
    };
}

CompileError CompileError::RMCommandNoAddress(size_t line) {
    return CompileError{
        "memory address not specified for RM format command",
        line,
    };
}

///------------------------------------RR------------------------------------///

CompileError CompileError::RRCommandNoReceiver(size_t line) {
    return CompileError{
        "receiver register not specified for RR format command",
        line,
    };
}

CompileError CompileError::RRCommandNoSource(size_t line) {
    return CompileError{
        "source register not specified for RR format command",
        line,
    };
}

CompileError CompileError::RRCommandNoModifier(size_t line) {
    return CompileError{
        "source modifier operand not specified for RR format command, "
        "specify 0 for no modification",
        line,
    };
}

///------------------------------------RI------------------------------------///

CompileError CompileError::RICommandNoRegister(size_t line) {
    return CompileError{
        "register not specified for RI format command",
        line,
    };
}

CompileError CompileError::RICommandNoImmediate(size_t line) {
    return CompileError{
        "immediate operand not specified for RI format command",
        line,
    };
}

///-------------------------------------J------------------------------------///

CompileError CompileError::JCommandNoAddress(size_t line) {
    return CompileError{
        "memory address not specified for J format command",
        line,
    };
}

///--------------------------------Extra words-------------------------------///

CompileError CompileError::ExtraWordsAfterEntrypoint(const std::string& extra,
                                                     size_t line) {
    std::ostringstream ss;
    ss << "the line starts with a valid "
       << std::quoted(syntax::kEntrypointDirective)
       << " directive, but has unexpected words at the end (starting from "
       << std::quoted(extra) << ")";
    return CompileError{ss.str(), line};
}

CompileError CompileError::ExtraWords(cmd::Format format,
                                      const std::string& extra,
                                      size_t line) {
    std::ostringstream ss;
    ss << "the line starts with a valid command (format "
       << cmd::kFormatToString.at(format)
       << "), but has unexpected words at the end (starting from "
       << std::quoted(extra) << ")";
    return CompileError{ss.str(), line};
}

}  // namespace karma::errors::compiler