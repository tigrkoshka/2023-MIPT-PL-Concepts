#include "compiler_errors.hpp"

#include <cstddef>  // for size_t
#include <iomanip>  // for quoted
#include <sstream>  // for ostringstream
#include <string>   // for string, to_string

#include "../specs/commands.hpp"
#include "../specs/constants.hpp"
#include "../specs/syntax.hpp"

namespace karma::errors::compiler {

namespace cmd = detail::specs::cmd;

namespace consts = detail::specs::consts;

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
    return {ss.str(), line};
}

InternalError InternalError::UnprocessedCommandFormat(cmd::Format format,
                                                      size_t line) {
    std::ostringstream ss;
    ss << "processing for command format " << cmd::kFormatToString.at(format)
       << " is not implemented";
    return {ss.str(), line};
}

InternalError InternalError::UnprocessedConstantType(consts::Type type,
                                                     size_t line) {
    std::ostringstream ss;
    ss << "processing for constant type " << consts::kTypeToName.at(type)
       << " is not implemented";
    return {ss.str(), line};
}

InternalError InternalError::EmptyWord(size_t line) {
    return {"the current word is empty", line};
}

////////////////////////////////////////////////////////////////////////////////
///                            Compilation errors                            ///
////////////////////////////////////////////////////////////////////////////////

///----------------------------------Labels----------------------------------///

CompileError CompileError::EmptyLabel(size_t line) {
    return {"label name must not be empty", line};
}

CompileError CompileError::LabelBeforeEntrypoint(
    size_t end_line,
    const std::string& latest_label,
    size_t latest_label_line) {
    std::ostringstream ss;
    ss << "label " << std::quoted(latest_label) << " is placed before the "
       << std::quoted(syntax::kEntrypointDirective) << " directive (at line "
       << end_line << "), but a label can only appear before a command";
    return {ss.str(), latest_label_line};
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
    return {ss.str(), line};
}

CompileError CompileError::LabelRedefinition(const std::string& label,
                                             size_t line,
                                             size_t previous_definition_line) {
    std::ostringstream ss;
    ss << "label " << std::quoted(label) << " redefinition at line " << line
       << ": previous definition was at line " << previous_definition_line;
    return {ss.str(), line};
}

CompileError CompileError::FileEndsWithLabel(const std::string& label,
                                             size_t line) {
    std::ostringstream ss;
    ss << "label " << std::quoted(label)
       << " is the last non-comment word in file";
    return {ss.str(), line};
}

CompileError CompileError::LabelStartsWithDigit(const std::string& label,
                                                size_t line) {
    std::ostringstream ss;
    ss << "label " << std::quoted(label) << " starts with a digit";
    return {ss.str(), line};
}

CompileError CompileError::InvalidLabelCharacter(char invalid,
                                                 const std::string& label,
                                                 size_t line) {
    std::ostringstream ss;
    ss << "label " << std::quoted(label) << " contains an invalid character: \'"
       << invalid << "\'";
    return {ss.str(), line};
}

CompileError CompileError::UndefinedLabel(const std::string& label,
                                          size_t line) {
    std::ostringstream ss;
    ss << "label " << std::quoted(label) << " is not defined";
    return {ss.str(), line};
}

///--------------------------------Entrypoint--------------------------------///

CompileError CompileError::NoEntrypoint() {
    return CompileError("did not encounter an entrypoint");
}

CompileError CompileError::SecondEntrypoint(size_t line,
                                            size_t entrypoint_line) {
    std::ostringstream ss;
    ss << "encountered second entrypoint (previous one on line "
       << entrypoint_line << ")";
    return {ss.str(), line};
}

CompileError CompileError::EntrypointWithoutAddress(size_t line) {
    return {"entrypoint address not specified", line};
}

///---------------------------------Constants--------------------------------///

CompileError CompileError::EmptyConstantValue(consts::Type type, size_t line) {
    std::ostringstream ss;
    ss << "empty value for a constant of type " << consts::kTypeToName.at(type);
    return {ss.str(), line};
}

CompileError CompileError::InvalidValue(consts::Type type,
                                        const std::string& value,
                                        size_t line) {
    std::ostringstream ss;
    ss << "invalid value for " << consts::kTypeToName.at(type) << " constant "
       << std::quoted(value);
    return {ss.str(), line};
}

CompileError CompileError::CharTooSmallForQuotes(const std::string& value,
                                                 size_t line) {
    std::ostringstream ss;
    ss << "a char constant must be surrounded by single quotes, instead got: "
       << std::quoted(value);
    return {ss.str(), line};
}

CompileError CompileError::CharNoStartQuote(const std::string& value,
                                            size_t line) {
    std::ostringstream ss;
    ss << "char constant value must start with a single quote, instead got: "
       << value;
    return {ss.str(), line};
}

CompileError CompileError::CharNoEndQuote(const std::string& value,
                                          size_t line) {
    std::ostringstream ss;
    ss << "char constant value must end with a single quote, instead got: "
       << value;
    return {ss.str(), line};
}

CompileError CompileError::StringTooSmallForQuotes(const std::string& value,
                                                   size_t line) {
    std::ostringstream ss;
    ss << "a string constant must be surrounded by double quotes, instead got: "
       << value;
    return {ss.str(), line};
}

CompileError CompileError::StringNoStartQuote(const std::string& value,
                                              size_t line) {
    std::ostringstream ss;
    ss << "string constant value must start with a double quote, instead got: "
       << value;
    return {ss.str(), line};
}

CompileError CompileError::StringNoEndQuote(const std::string& value,
                                            size_t line) {
    std::ostringstream ss;
    ss << "string constant value must end with a double quote, instead got: "
       << value;
    return {ss.str(), line};
}

///----------------------------------Command---------------------------------///

CompileError CompileError::UnknownCommand(const std::string& command,
                                          size_t line) {
    std::ostringstream ss;
    ss << "unknown command " << std::quoted(command);
    return {ss.str(), line};
}

///---------------------------------Register---------------------------------///

CompileError CompileError::UnknownRegister(const std::string& reg,
                                           size_t line) {
    std::ostringstream ss;
    ss << "unknown register " << std::quoted(reg);
    return {ss.str(), line};
}

///----------------------------------Address---------------------------------///

CompileError CompileError::AddressNegative(const std::string& address,
                                           size_t line) {
    std::ostringstream ss;
    ss << "the address operand " << std::quoted(address, ')')
       << " must not be negative";
    return {ss.str(), line};
}

CompileError CompileError::AddressOutOfMemory(const std::string& address,
                                              size_t line) {
    std::ostringstream ss;
    ss << "the address operand " << std::quoted(address, ')')
       << " exceeds the memory size";
    return {ss.str(), line};
}

///---------------------------------Immediate--------------------------------///

CompileError CompileError::ImmediateNotANumber(const std::string& immediate,
                                               size_t line) {
    std::ostringstream ss;
    ss << "the immediate operand is not a number: " << std::quoted(immediate);
    return {ss.str(), line};
}

CompileError CompileError::ImmediateLessThanMin(int32_t min,
                                                const std::string& immediate,
                                                size_t line) {
    std::ostringstream ss;
    ss << "the immediate operand is less than the allowed minimum (" << min
       << "): " << immediate;
    return {ss.str(), line};
}

CompileError CompileError::ImmediateMoreThanMax(int32_t max,
                                                const std::string& immediate,
                                                size_t line) {
    std::ostringstream ss;
    ss << "the immediate operand is less than the allowed maximum (" << max
       << "): " << immediate;
    return {ss.str(), line};
}

CompileError CompileError::ImmediateOutOfRange(const std::string& immediate,
                                               size_t line) {
    std::ostringstream ss;
    ss << "the immediate operand is out of range " << immediate;
    return {ss.str(), line};
}

///------------------------------------RM------------------------------------///

CompileError CompileError::RMCommandNoRegister(size_t line) {
    return {"register not specified for RM format command", line};
}

CompileError CompileError::RMCommandNoAddress(size_t line) {
    return {"memory address not specified for RM format command", line};
}

///------------------------------------RR------------------------------------///

CompileError CompileError::RRCommandNoReceiver(size_t line) {
    return {"receiver register not specified for RR format command", line};
}

CompileError CompileError::RRCommandNoSource(size_t line) {
    return {"source register not specified for RR format command", line};
}

CompileError CompileError::RRCommandNoModifier(size_t line) {
    return {
        "source modifier operand not specified for RR format command, "
        "specify 0 for no modification",
        line,
    };
}

///------------------------------------RI------------------------------------///

CompileError CompileError::RICommandNoRegister(size_t line) {
    return {"register not specified for RI format command", line};
}

CompileError CompileError::RICommandNoImmediate(size_t line) {
    return {"immediate operand not specified for RI format command", line};
}

///-------------------------------------J------------------------------------///

CompileError CompileError::JCommandNoAddress(size_t line) {
    return {"memory address not specified for J format command", line};
}

///--------------------------------Extra words-------------------------------///

CompileError CompileError::ExtraWordsAfterEntrypoint(const std::string& extra,
                                                     size_t line) {
    std::ostringstream ss;
    ss << "the line starts with a valid "
       << std::quoted(syntax::kEntrypointDirective)
       << " directive, but has unexpected words at the end (starting from "
       << std::quoted(extra) << ")";
    return {ss.str(), line};
}

CompileError CompileError::ExtraWordsAfterConstant(consts::Type type,
                                                   const std::string& extra,
                                                   size_t line) {
    std::ostringstream ss;
    ss << "the line starts with a valid constant (type "
       << consts::kTypeToName.at(type)
       << "), but has unexpected words at the end (starting from "
       << std::quoted(extra) << ")";
    return {ss.str(), line};
}

CompileError CompileError::ExtraWordsAfterCommand(cmd::Format fmt,
                                                  const std::string& extra,
                                                  size_t line) {
    std::ostringstream ss;
    ss << "the line starts with a valid command (format "
       << cmd::kFormatToString.at(fmt)
       << "), but has unexpected words at the end (starting from "
       << std::quoted(extra) << ")";
    return {ss.str(), line};
}

}  // namespace karma::errors::compiler