#include "compiler.hpp"

#include <algorithm>  // for min
#include <cctype>     // for isalnum, isdigit
#include <cstddef>    // for size_t
#include <exception>  // for exception
#include <fstream>    // for ofstream
#include <iomanip>    // for quoted
#include <iostream>   // for cout
#include <stdexcept>  // for runtime_error
#include <string>     // for string

#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/exec.hpp"
#include "specs/syntax.hpp"

namespace karma {

namespace syntax = detail::specs::syntax;

namespace cmd  = detail::specs::cmd;
namespace args = cmd::args;

namespace arch  = detail::specs::arch;
namespace types = arch::types;

namespace exec = detail::specs::exec;

////////////////////////////////////////////////////////////////////////////////
///                                  Errors                                  ///
////////////////////////////////////////////////////////////////////////////////

struct Compiler::Error : std::runtime_error {
   private:
    friend void Compiler::CompileImpl(std::istream& code,
                                      const std::string& exec_path);

   protected:
    explicit Error(const std::string& message)
        : std::runtime_error(message) {}

   public:
    Error(const Error&)            = default;
    Error& operator=(const Error&) = default;
    Error(Error&&)                 = default;
    Error& operator=(Error&&)      = default;
    ~Error() override              = default;
};

struct Compiler::InternalError : Error {
   private:
    friend void Compiler::Compile(std::istream& code,
                                  const std::string& exec_path);

   private:
    explicit InternalError(const std::string& message)
        : Error("internal compiler error: " + message) {}

    InternalError(const std::string& message, size_t line)
        : Error("internal compiler error at line " + std::to_string(line) +
                ": " + message) {}

   public:
    InternalError(const InternalError&)            = default;
    InternalError& operator=(const InternalError&) = default;
    InternalError(InternalError&&)                 = default;
    InternalError& operator=(InternalError&&)      = default;
    ~InternalError() override                      = default;

    static InternalError UnknownCommandCode(cmd::Code command_code,
                                            size_t line) {
        std::ostringstream ss;
        ss << "unknown command code " << command_code;
        return InternalError{ss.str(), line};
    }

    static InternalError UnknownCommandFormat(cmd::Format format, size_t line) {
        std::ostringstream ss;
        ss << "unknown command format " << format;
        return InternalError{ss.str(), line};
    }

    static InternalError EmptyWord(size_t line) {
        return InternalError{"the current word is empty", line};
    }
};

struct Compiler::CompileError : Error {
   private:
    explicit CompileError(const std::string& message)
        : Error("compile error: " + message) {}

    explicit CompileError(const std::string& message, size_t line)
        : Error("compile error at line " + std::to_string(line) + ": " +
                message) {}

   public:
    CompileError(const CompileError&)            = default;
    CompileError& operator=(const CompileError&) = default;
    CompileError(CompileError&&)                 = default;
    CompileError& operator=(CompileError&&)      = default;
    ~CompileError() override                     = default;

    // command

    static CompileError UnknownCommand(const std::string& command,
                                       size_t line) {
        std::ostringstream ss;
        ss << "unknown command " << std::quoted(command);
        return CompileError{ss.str(), line};
    }

    // entrypoint

    static CompileError NoEntrypoint() {
        return CompileError("did not encounter an entrypoint");
    }

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    static CompileError SecondEntrypoint(size_t line, size_t entrypoint_line) {
        std::ostringstream ss;
        ss << "encountered second entrypoint (previous one on line "
           << entrypoint_line << ")";
        return CompileError{ss.str(), line};
    }

    static CompileError EntrypointWithoutAddress(size_t line) {
        return CompileError{"entrypoint address not specified", line};
    }

    // labels

    static CompileError EmptyLabel(size_t line) {
        return CompileError("label name must not be empty", line);
    }

    static CompileError LabelBeforeEntrypoint(size_t end_line,
                                              const std::string& latest_label,
                                              size_t latest_label_line) {
        std::ostringstream ss;
        ss << "label " << std::quoted(latest_label) << " is placed before the "
           << std::quoted(syntax::kEntrypointDirective)
           << " directive (at line " << end_line
           << "), but a label can only appear before a command";
        return CompileError{ss.str(), latest_label_line};
    }

    static CompileError ConsecutiveLabels(const std::string& label,
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

    static CompileError FileEndsWithLabel(const std::string& label,
                                          size_t line) {
        std::ostringstream ss;
        ss << "label " << std::quoted(label)
           << " is the last non-comment word in file";
        return CompileError{ss.str(), line};
    }

    static CompileError LabelStartsWithDigit(const std::string& label,
                                             size_t line) {
        std::ostringstream ss;
        ss << "label " << std::quoted(label) << " starts with a digit";
        return CompileError{ss.str(), line};
    }

    static CompileError InvalidLabelCharacter(char invalid,
                                              const std::string& label,
                                              size_t line) {
        std::ostringstream ss;
        ss << "label " << std::quoted(label)
           << " contains an invalid character: \'" << invalid
           << "\' (only latin letters and digits are allowed)";
        return CompileError{ss.str(), line};
    }

    static CompileError UndefinedLabel(const std::string& label, size_t line) {
        std::ostringstream ss;
        ss << "label " << std::quoted(label) << " is not defined";
        return CompileError{ss.str(), line};
    }

    // register

    static CompileError UnknownRegister(const std::string& reg, size_t line) {
        std::ostringstream ss;
        ss << "unknown register " << std::quoted(reg);
        return CompileError{ss.str(), line};
    }

    // address

    static CompileError AddressNegative(const std::string& address,
                                        size_t line) {
        std::ostringstream ss;
        ss << "the address operand " << std::quoted(address, ')')
           << " must not be negative";
        return CompileError{ss.str(), line};
    }

    static CompileError AddressOutOfMemory(const std::string& address,
                                           size_t line) {
        std::ostringstream ss;
        ss << "the address operand " << std::quoted(address, ')')
           << " exceeds the memory size";
        return CompileError{ss.str(), line};
    }

    // immediate

    static CompileError ImmediateNotANumber(const std::string& immediate,
                                            size_t line) {
        std::ostringstream ss;
        ss << "the immediate operand is not a number: "
           << std::quoted(immediate);
        return CompileError{ss.str(), line};
    }

    static CompileError ImmediateLessThanMin(int32_t min,
                                             const std::string& immediate,
                                             size_t line) {
        std::ostringstream ss;
        ss << "the immediate operand is less than the allowed minimum (" << min
           << "): " << immediate;
        return CompileError{ss.str(), line};
    }

    static CompileError ImmediateMoreThanMax(int32_t max,
                                             const std::string& immediate,
                                             size_t line) {
        std::ostringstream ss;
        ss << "the immediate operand is less than the allowed maximum (" << max
           << "): " << immediate;
        return CompileError{ss.str(), line};
    }

    static CompileError ImmediateOutOfRange(const std::string& immediate,
                                            size_t line) {
        std::ostringstream ss;
        ss << "the immediate operand is out of range " << immediate;
        return CompileError{ss.str(), line};
    }

    // RM

    static CompileError RMCommandNoRegister(size_t line) {
        return CompileError{
            "register not specified for RM format command",
            line,
        };
    }

    static CompileError RMCommandNoAddress(size_t line) {
        return CompileError{
            "memory address not specified for RM format command",
            line,
        };
    }

    // RR

    static CompileError RRCommandNoReceiver(size_t line) {
        return CompileError{
            "receiver register not specified for RR format command",
            line,
        };
    }

    static CompileError RRCommandNoSource(size_t line) {
        return CompileError{
            "source register not specified for RR format command",
            line,
        };
    }

    static CompileError RRCommandNoModifier(size_t line) {
        return CompileError{
            "source modifier operand not specified for RR format command, "
            "specify 0 for no modification",
            line,
        };
    }

    // RI

    static CompileError RICommandNoRegister(size_t line) {
        return CompileError{
            "register not specified for RI format command",
            line,
        };
    }

    static CompileError RICommandNoImmediate(size_t line) {
        return CompileError{
            "immediate operand not specified for RI format command",
            line,
        };
    }

    // J

    static CompileError JCommandNoAddress(size_t line) {
        return CompileError{
            "memory address not specified for J format command",
            line,
        };
    }

    // Extra words

    static CompileError ExtraWordsAfterEntrypoint(const std::string& extra,
                                                  size_t line) {
        std::ostringstream ss;
        ss << "the line starts with a valid "
           << std::quoted(syntax::kEntrypointDirective)
           << " directive, but has unexpected words at the end (starting from "
           << std::quoted(extra) << ")";
        return CompileError{ss.str(), line};
    }

    static CompileError ExtraWords(cmd::Format format,
                                   const std::string& extra,
                                   size_t line) {
        std::ostringstream ss;
        ss << "the line starts with a valid command (format "
           << cmd::kFormatToString.at(format)
           << "), but has unexpected words at the end (starting from "
           << std::quoted(extra) << ")";
        return CompileError{ss.str(), line};
    }
};

////////////////////////////////////////////////////////////////////////////////
///                               Word parsing                               ///
////////////////////////////////////////////////////////////////////////////////

bool Compiler::TryProcessEntrypoint() {
    if (curr_word_ != syntax::kEntrypointDirective) {
        return false;
    }

    if (seen_entrypoint_) {
        throw CompileError::SecondEntrypoint(line_number_, entrypoint_line_);
    }

    if (latest_word_was_label_) {
        throw CompileError::LabelBeforeEntrypoint(line_number_,
                                                  latest_label_,
                                                  latest_label_line_);
    }

    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::EntrypointWithoutAddress(line_number_);
    }

    entrypoint_            = GetAddress(true);
    entrypoint_line_       = line_number_;
    seen_entrypoint_       = true;
    latest_word_was_label_ = false;

    if (curr_line_ >> curr_word_) {
        throw CompileError::ExtraWordsAfterEntrypoint(curr_word_, line_number_);
    }

    return true;
}

void Compiler::CheckLabel() const {
    if (curr_word_.empty()) {
        throw CompileError::EmptyLabel(line_number_);
    }

    if (std::isdigit(curr_word_[0]) != 0) {
        throw CompileError::LabelStartsWithDigit(curr_word_, line_number_);
    }

    for (char symbol : curr_word_) {
        if (std::isalnum(symbol) == 0) {
            throw CompileError::InvalidLabelCharacter(symbol,
                                                      curr_word_,
                                                      line_number_);
        }
    }
}

bool Compiler::TryProcessLabel() {
    if (curr_word_.back() != syntax::kLabelEnd) {
        return false;
    }

    curr_word_.resize(curr_word_.size() - 1);

    CheckLabel();

    if (latest_word_was_label_) {
        throw CompileError::ConsecutiveLabels(curr_word_,
                                              line_number_,
                                              latest_label_,
                                              latest_label_line_);
    }

    latest_word_was_label_         = true;
    latest_label_                  = curr_word_;
    latest_label_line_             = line_number_;
    label_definitions_[curr_word_] = static_cast<types::Word>(command_number_);

    return true;
}

cmd::CodeFormat Compiler::GetCommand() {
    if (!cmd::kCommandToCode.contains(curr_word_)) {
        throw CompileError::UnknownCommand(curr_word_, line_number_);
    }

    cmd::Code code = cmd::kCommandToCode.at(curr_word_);

    if (!cmd::kCodeToFormat.contains(code)) {
        throw InternalError::UnknownCommandCode(code, line_number_);
    }

    cmd::Format format = cmd::kCodeToFormat.at(code);

    latest_word_was_label_ = false;

    return {code, format};
}

args::Register Compiler::GetRegister() const {
    if (curr_word_.empty()) {
        throw InternalError::EmptyWord(line_number_);
    }

    if (!arch::kRegisterToNum.contains(curr_word_)) {
        throw CompileError::UnknownRegister(curr_word_, line_number_);
    }

    return arch::kRegisterToNum.at(curr_word_);
}

args::Immediate Compiler::GetImmediate(size_t bit_size) const {
    if (curr_word_.empty()) {
        throw InternalError::EmptyWord(line_number_);
    }

    const int32_t min = -static_cast<int32_t>(1u << (bit_size - 1));
    const int32_t max = static_cast<int32_t>(1u << (bit_size - 1)) - 1;

    try {
        size_t pos{};
        int32_t operand = std::stoi(curr_word_, &pos, 0);

        if (pos != curr_word_.size()) {
            throw CompileError::ImmediateNotANumber(curr_word_, line_number_);
        }

        if (operand < min) {
            throw CompileError::ImmediateLessThanMin(min,
                                                     curr_word_,
                                                     line_number_);
        }

        if (operand > max) {
            throw CompileError::ImmediateMoreThanMax(max,
                                                     curr_word_,
                                                     line_number_);
        }

        return *reinterpret_cast<uint32_t*>(&operand);
    } catch (const std::invalid_argument&) {
        throw CompileError::ImmediateNotANumber(curr_word_, line_number_);
    } catch (const std::out_of_range&) {
        throw CompileError::ImmediateOutOfRange(curr_word_, line_number_);
    }
}

args::Address Compiler::GetAddress(bool is_entrypoint) {
    if (curr_word_.empty()) {
        throw InternalError::EmptyWord(line_number_);
    }

    try {
        size_t pos{};
        int32_t operand = std::stoi(curr_word_, &pos, 0);

        if (pos != curr_word_.size()) {
            // this means that there is a number in the beginning of word,
            // but the whole word is not a number
            //
            // assume the programmer meant it as a label,
            // but a label cannot start with a digit
            //
            // NOTE: we do allow '0', '0x' and '0X' prefixed numbers,
            //       but in all those cases word still starts with a digit ('0')
            //       and thus is not a valid label
            throw CompileError::LabelStartsWithDigit(curr_word_, line_number_);
        }

        if (operand < 0) {
            throw CompileError::AddressNegative(curr_word_, line_number_);
        }

        auto addr = static_cast<args::Address>(operand);
        if (addr > arch::kMemorySize) {
            throw CompileError::AddressOutOfMemory(curr_word_, line_number_);
        }

        return addr;
    } catch (const std::invalid_argument&) {
        // this implies that the word does not start with a digit,
        // so we assume it's a label
        CheckLabel();

        if (is_entrypoint) {
            entrypoint_label_ = curr_word_;
        } else {
            label_usages_[curr_word_].emplace_back(line_number_,
                                                   command_number_);
        }
        return 0;
    } catch (const std::out_of_range&) {
        throw CompileError::AddressOutOfMemory(curr_word_, line_number_);
    }
}

////////////////////////////////////////////////////////////////////////////////
///                               Line parsing                               ///
////////////////////////////////////////////////////////////////////////////////

args::RMArgs Compiler::GetRMOperands() {
    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::RMCommandNoRegister(line_number_);
    }

    args::Register reg = GetRegister();

    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::RMCommandNoAddress(line_number_);
    }

    return {reg, GetAddress()};
}

args::RRArgs Compiler::GetRROperands() {
    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::RRCommandNoReceiver(line_number_);
    }

    args::Register recv = GetRegister();

    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::RRCommandNoSource(line_number_);
    }

    args::Register src = GetRegister();

    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::RRCommandNoModifier(line_number_);
    }

    return {recv, src, GetImmediate(args::kModSize)};
}

args::RIArgs Compiler::GetRIOperands() {
    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::RICommandNoRegister(line_number_);
    }

    args::Register reg = GetRegister();

    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::RICommandNoImmediate(line_number_);
    }

    return {reg, GetImmediate(args::kImmSize)};
}

args::JArgs Compiler::GetJOperands() {
    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::JCommandNoAddress(line_number_);
    }

    return GetAddress();
}

void Compiler::ProcessCurrLine() {
    if (!(curr_line_ >> curr_word_)) {
        return;
    }

    if (TryProcessEntrypoint()) {
        return;
    }

    if (TryProcessLabel() && !(curr_line_ >> curr_word_)) {
        return;
    }

    ++command_number_;

    auto [code, format] = GetCommand();
    switch (format) {
        case cmd::format::RM: {
            auto [reg, addr] = GetRMOperands();
            compiled_.push_back((code << cmd::format::kCodeShift) |
                                (reg << cmd::format::kRecvShift) | addr);
            break;
        }

        case cmd::format::RR: {
            auto [recv, src, mod] = GetRROperands();
            compiled_.push_back((code << cmd::format::kCodeShift) |
                                (recv << cmd::format::kRecvShift) |
                                (src << cmd::format::kSrcShift) | mod);
            break;
        }

        case cmd::format::RI: {
            auto [recv, imm] = GetRIOperands();
            compiled_.push_back((code << cmd::format::kCodeShift) |
                                (recv << cmd::format::kRecvShift) | imm);
            break;
        }

        case cmd::format::J: {
            auto [addr] = GetJOperands();
            compiled_.push_back((code << cmd::format::kCodeShift) | addr);

            break;
        }

        default: {
            throw InternalError::UnknownCommandFormat(format, line_number_);
        }
    }

    if (curr_line_ >> curr_word_) {
        CompileError::ExtraWords(format, curr_word_, line_number_);
    }
}

////////////////////////////////////////////////////////////////////////////////
///                            Labels substitution                           ///
////////////////////////////////////////////////////////////////////////////////

void Compiler::FillLabels() {
    for (const auto& [label, usages] : label_usages_) {
        if (!label_definitions_.contains(label)) {
            // use usages[0], because it is the first occurrence
            // of the label (we store label usages while parsing
            // the file line by line)
            throw CompileError::UndefinedLabel(label, usages[0].first);
        }

        arch::Address address = label_definitions_.at(label);
        for (auto [_, n_cmd] : usages) {
            // the address always occupies the last
            // bits of the command binary
            compiled_[n_cmd] |= address;
        }
    }

    if (!entrypoint_label_.empty()) {
        if (!label_definitions_.contains(entrypoint_label_)) {
            throw CompileError::UndefinedLabel(entrypoint_label_,
                                               entrypoint_line_);
        }

        entrypoint_ = label_definitions_.at(entrypoint_label_);
    }
}

////////////////////////////////////////////////////////////////////////////////
///                                  Compile                                 ///
////////////////////////////////////////////////////////////////////////////////

void Compiler::DoCompile(std::istream& code) {
    for (std::string line; std::getline(code, line); ++line_number_) {
        // ignore comments
        line.resize(std::min(line.find(syntax::kCommentSep), line.size()));

        curr_line_ = std::istringstream(std::move(line));
        ProcessCurrLine();
    }

    if (latest_word_was_label_) {
        throw CompileError::FileEndsWithLabel(latest_label_,
                                              latest_label_line_);
    }

    if (!seen_entrypoint_) {
        throw CompileError::NoEntrypoint();
    }

    FillLabels();
}

void Compiler::CompileImpl(std::istream& code, const std::string& exec_path) {
    DoCompile(code);

    std::ofstream binary(exec_path, std::ios::binary | std::ios::trunc);

    // check that the file was found
    if (binary.fail()) {
        throw Error("failed to open exec file for writing");
    }

    auto write_word = [&binary](types::Word word) {
        binary.write(reinterpret_cast<char*>(&word), 4);
    };

    // intro string
    binary << "ThisIsKarmaExec" << '\0';

    // code size (in bytes)
    write_word(static_cast<types::Word>(compiled_.size()) *
               static_cast<types::Word>(types::kWordSize));

    // const size (in bytes)
    write_word(0);

    // data size (in bytes)
    write_word(0);

    // entrypoint address
    write_word(entrypoint_);

    // initial stack pointer
    write_word(static_cast<types::Word>(arch::kMemorySize - 1));

    // target processor ID
    write_word(exec::kProcessorID);

    // empty
    binary << std::string(exec::kCodeSegmentPos - exec::kMetaInfoEndPos, '0');

    // code
    for (auto command : compiled_) {
        write_word(command);
    }
}

void Compiler::Compile(std::istream& code, const std::string& exec_path) {
    try {
        CompileImpl(code, exec_path);
    } catch (const Error& e) {
        std::cout << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << InternalError(e.what()).what() << std::endl;
    } catch (...) {
        std::cout << InternalError("unexpected exception").what() << std::endl;
    }
}

}  // namespace karma
