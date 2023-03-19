#include "errors.hpp"

#include <iomanip>  // for quoted
#include <sstream>  // for ostringstream
#include <string>   // for string, to_string

#include "specs/commands.hpp"
#include "specs/constants.hpp"
#include "specs/syntax.hpp"

namespace karma::errors::compiler {

using IE = InternalError;
using CE = CompileError;

namespace cmd    = detail::specs::cmd;
namespace consts = detail::specs::consts;
namespace syntax = detail::specs::syntax;

////////////////////////////////////////////////////////////////////////////////
///                              Internal errors                             ///
////////////////////////////////////////////////////////////////////////////////

IE IE::Builder::RepeatedOpenFile(const std::string& path) {
    std::ostringstream ss;
    ss << "attempt to open file " << std::quoted(path)
       << ", which was already opened";
    return IE{ss.str()};
}

IE IE::Builder::CloseUnopenedFile(const std::string& path) {
    std::ostringstream ss;
    ss << "attempt to close file " << std::quoted(path)
       << ", which was not opened";
    return IE{ss.str()};
}

IE IE::Builder::FormatNotFound(cmd::Code command_code, Where where) {
    std::ostringstream ss;
    ss << "did not find format for command with code " << command_code;
    return {ss.str(), where};
}

IE IE::Builder::UnprocessedCommandFormat(cmd::Format format, Where where) {
    std::ostringstream ss;
    ss << "processing for command format " << cmd::kFormatToString.at(format)
       << " is not implemented";
    return {ss.str(), where};
}

IE IE::Builder::UnprocessedConstantType(consts::Type type, Where where) {
    std::ostringstream ss;
    ss << "processing for constant type " << consts::kTypeToName.at(type)
       << " is not implemented";
    return {ss.str(), where};
}

IE IE::Builder::EmptyWord(Where where) {
    return {"the current word is empty", where};
}

IE IE::Builder::NoEntrypointPosInLabelSubstitution() {
    return IE{
        "no entrypoint position is defined during the labels substitution "
        "stage, although the entrypoint address is present, because otherwise "
        "a compile error would have been thrown on an earlier stage"};
}

////////////////////////////////////////////////////////////////////////////////
///                            Compilation errors                            ///
////////////////////////////////////////////////////////////////////////////////

///-----------------------------------File-----------------------------------///

CE CE::Builder::FailedToOpen(const std::string& path) {
    std::ostringstream ss;
    ss << "failed to open " << std::quoted(path);
    return CE{ss.str()};
}

///---------------------------------Includes---------------------------------///

CE CE::Builder::IncludeNoFilename(Where where) {
    return {"filename not specified for the include directive", where};
}

///----------------------------------Labels----------------------------------///

CE CE::Builder::EmptyLabel(Where where) {
    return {"label name must not be empty", where};
}

CE CE::Builder::LabelBeforeEntrypoint(Where entry, Label label) {
    std::ostringstream ss;
    ss << "label " << std::quoted(label.value) << " is placed before the "
       << std::quoted(syntax::kEntrypointDirective) << " directive " << entry;
    return {ss.str(), label.where};
}

CE CE::Builder::ConsecutiveLabels(Label curr, Label prev) {
    std::ostringstream ss;
    ss << "label " << std::quoted(curr.value)
       << " is not separated from the previous one by at least one command or "
       << "constant, the previous label: " << std::quoted(prev.value)
       << prev.where;
    return {ss.str(), curr.where};
}

CE CE::Builder::LabelRedefinition(Label label, Where previous_pos) {
    std::ostringstream ss;
    ss << "label " << std::quoted(label.value)
       << " redefinition,\n\nprevious definition was " << previous_pos;
    return {ss.str(), label.where};
}

CE CE::Builder::FileEndsWithLabel(Label label) {
    std::ostringstream ss;
    ss << "label " << std::quoted(label.value)
       << " is the last non-comment word in file";
    return {ss.str(), label.where};
}

CE CE::Builder::LabelStartsWithDigit(Label label) {
    std::ostringstream ss;
    ss << "label " << std::quoted(label.value) << " starts with a digit";
    return {ss.str(), label.where};
}

CE CE::Builder::InvalidLabelCharacter(char invalid, Label label) {
    std::ostringstream ss;
    ss << "label " << std::quoted(label.value)
       << " contains an invalid character: \'" << invalid << "\'";
    return {ss.str(), label.where};
}

CE CE::Builder::UndefinedLabel(Label label) {
    std::ostringstream ss;
    ss << "label " << std::quoted(label.value) << " is not defined";
    return {ss.str(), label.where};
}

///--------------------------------Entrypoint--------------------------------///

CE CE::Builder::NoEntrypoint() {
    return CompileError("did not encounter an entrypoint");
}

CE CE::Builder::SecondEntrypoint(Where curr, Where prev) {
    std::ostringstream ss;
    ss << "encountered second entrypoint, previous one " << prev;
    return {ss.str(), curr};
}

CE CE::Builder::EntrypointWithoutAddress(Where where) {
    return {"entrypoint address not specified", where};
}

///---------------------------------Constants--------------------------------///

CE CE::Builder::EmptyConstValue(consts::Type type, Where where) {
    std::ostringstream ss;
    ss << "empty value for a constant of type " << consts::kTypeToName.at(type);
    return {ss.str(), where};
}

CE CE::Builder::InvalidConstValue(consts::Type type, Value value) {
    std::ostringstream ss;
    ss << "invalid value for " << consts::kTypeToName.at(type) << " constant "
       << std::quoted(value.value);
    return {ss.str(), value.where};
}

CE CE::Builder::CharTooSmallForQuotes(Value value) {
    std::ostringstream ss;
    ss << "a char constant must be surrounded by single quotes, instead got: "
       << std::quoted(value.value);
    return {ss.str(), value.where};
}

CE CE::Builder::CharNoStartQuote(Value value) {
    std::ostringstream ss;
    ss << "char constant value must start with a single quote, instead got: "
       << value.value;
    return {ss.str(), value.where};
}

CE CE::Builder::CharNoEndQuote(Value value) {
    std::ostringstream ss;
    ss << "char constant value must end with a single quote, instead got: "
       << value.value;
    return {ss.str(), value.where};
}

CE CE::Builder::StringTooSmallForQuotes(Value value) {
    std::ostringstream ss;
    ss << "a string constant must be surrounded by double quotes, instead got: "
       << value.value;
    return {ss.str(), value.where};
}

CE CE::Builder::StringNoStartQuote(Value value) {
    std::ostringstream ss;
    ss << "string constant value must start with a double quote, instead got: "
       << value.value;
    return {ss.str(), value.where};
}

CE CE::Builder::StringNoEndQuote(Value value) {
    std::ostringstream ss;
    ss << "string constant value must end with a double quote, instead got: "
       << value.value;
    return {ss.str(), value.where};
}

///----------------------------------Command---------------------------------///

CE CE::Builder::UnknownCommand(Command command) {
    std::ostringstream ss;
    ss << "unknown command " << std::quoted(command.value);
    return {ss.str(), command.where};
}

///---------------------------------Register---------------------------------///

CE CE::Builder::UnknownRegister(Register reg) {
    std::ostringstream ss;
    ss << "unknown register " << std::quoted(reg.value);
    return {ss.str(), reg.where};
}

///----------------------------------Address---------------------------------///

CE CE::Builder::AddressNegative(Address address) {
    std::ostringstream ss;
    ss << "the address operand " << std::quoted(address.value, ')')
       << " must not be negative";
    return {ss.str(), address.where};
}

CE CE::Builder::AddressOutOfMemory(Address address) {
    std::ostringstream ss;
    ss << "the address operand " << std::quoted(address.value, ')')
       << " exceeds the memory size";
    return {ss.str(), address.where};
}

///---------------------------------Immediate--------------------------------///

CE CE::Builder::ImmediateNotANumber(Immediate immediate) {
    std::ostringstream ss;
    ss << "the immediate operand is not a number: "
       << std::quoted(immediate.value);
    return {ss.str(), immediate.where};
}

CE CE::Builder::ImmediateLessThanMin(int32_t min, Immediate immediate) {
    std::ostringstream ss;
    ss << "the immediate operand is less than the allowed minimum (" << min
       << "): " << immediate.value;
    return {ss.str(), immediate.where};
}

CE CE::Builder::ImmediateMoreThanMax(int32_t max, Immediate immediate) {
    std::ostringstream ss;
    ss << "the immediate operand is less than the allowed maximum (" << max
       << "): " << immediate.value;
    return {ss.str(), immediate.where};
}

CE CE::Builder::ImmediateOutOfRange(Immediate immediate) {
    std::ostringstream ss;
    ss << "the immediate operand is out of range " << immediate.value;
    return {ss.str(), immediate.where};
}

///------------------------------------RM------------------------------------///

CE CE::Builder::RMNoRegister(Where where) {
    return {"register not specified for RM format command", where};
}

CE CE::Builder::RMNoAddress(Where where) {
    return {"memory address not specified for RM format command", where};
}

///------------------------------------RR------------------------------------///

CE CE::Builder::RRNoReceiver(Where where) {
    return {"receiver register not specified for RR format command", where};
}

CE CE::Builder::RRNoSource(Where where) {
    return {"source register not specified for RR format command", where};
}

CE CE::Builder::RRNoModifier(Where where) {
    return {
        "source modifier operand not specified for RR format command, "
        "specify 0 for no modification",
        where,
    };
}

///------------------------------------RI------------------------------------///

CE CE::Builder::RINoRegister(Where where) {
    return {"register not specified for RI format command", where};
}

CE CE::Builder::RINoImmediate(Where where) {
    return {"immediate operand not specified for RI format command", where};
}

///-------------------------------------J------------------------------------///

CE CE::Builder::JNoAddress(Where where) {
    return {"memory address not specified for J format command", where};
}

///--------------------------------Extra words-------------------------------///

CE CE::Builder::ExtraAfterEntrypoint(Extra extra) {
    std::ostringstream ss;
    ss << "the line starts with a valid "
       << std::quoted(syntax::kEntrypointDirective)
       << " directive, but has unexpected words at the end (starting from "
       << std::quoted(extra.value) << ")";
    return {ss.str(), extra.where};
}

CE CE::Builder::ExtraAfterConstant(consts::Type type, Extra extra) {
    std::ostringstream ss;
    ss << "the line starts with a valid constant (type "
       << consts::kTypeToName.at(type)
       << "), but has unexpected words at the end (starting from "
       << std::quoted(extra.value) << ")";
    return {ss.str(), extra.where};
}

CE CE::Builder::ExtraAfterCommand(cmd::Format fmt, Extra extra) {
    std::ostringstream ss;
    ss << "the line starts with a valid command (format "
       << cmd::kFormatToString.at(fmt)
       << "), but has unexpected words at the end (starting from "
       << std::quoted(extra.value) << ")";
    return {ss.str(), extra.where};
}

}  // namespace karma::errors::compiler
