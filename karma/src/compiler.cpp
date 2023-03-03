#include "compiler.hpp"

#include <algorithm>    // for min
#include <cctype>       // for isalnum, isdigit
#include <cstddef>      // for size_t
#include <exception>    // for exception
#include <filesystem>   // for path
#include <fstream>      // for ifstream
#include <iostream>     // for cout, ios
#include <type_traits>  // for make_unsigned_t
#include <utility>      // for move

#include "errors/compiler_errors.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/exec.hpp"
#include "specs/syntax.hpp"

namespace karma {

using namespace errors::compiler;  // NOLINT(google-build-using-namespace)

namespace syntax = detail::specs::syntax;

namespace cmd  = detail::specs::cmd;
namespace args = cmd::args;

namespace arch  = detail::specs::arch;
namespace types = arch::types;

namespace exec = detail::specs::exec;

////////////////////////////////////////////////////////////////////////////////
///                               Word parsing                               ///
////////////////////////////////////////////////////////////////////////////////

bool Compiler::Impl::TryProcessEntrypoint() {
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

void Compiler::Impl::CheckLabel() const {
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

bool Compiler::Impl::TryProcessLabel() {
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

cmd::CodeFormat Compiler::Impl::GetCommand() {
    if (!cmd::kNameToCode.contains(curr_word_)) {
        throw CompileError::UnknownCommand(curr_word_, line_number_);
    }

    cmd::Code code = cmd::kNameToCode.at(curr_word_);

    if (!cmd::kCodeToFormat.contains(code)) {
        throw InternalError::FormatNotFound(code, line_number_);
    }

    cmd::Format format = cmd::kCodeToFormat.at(code);

    latest_word_was_label_ = false;

    return {code, format};
}

args::Register Compiler::Impl::GetRegister() const {
    if (curr_word_.empty()) {
        throw InternalError::EmptyWord(line_number_);
    }

    if (!arch::kRegisterNameToNum.contains(curr_word_)) {
        throw CompileError::UnknownRegister(curr_word_, line_number_);
    }

    return arch::kRegisterNameToNum.at(curr_word_);
}

args::Immediate Compiler::Impl::GetImmediate(size_t bit_size) const {
    if (curr_word_.empty()) {
        throw InternalError::EmptyWord(line_number_);
    }

    using Int  = args::Immediate;
    using Uint = std::make_unsigned_t<Int>;

    const auto min_modulo = static_cast<Uint>(1)
                            << static_cast<Uint>(bit_size - 1ull);

    const auto min = -static_cast<Int>(min_modulo);
    const auto max = static_cast<Int>(min_modulo - static_cast<Uint>(1));

    try {
        size_t pos{};
        Int operand = std::stoi(curr_word_, &pos, 0);

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

        return operand;
    } catch (const std::invalid_argument&) {
        throw CompileError::ImmediateNotANumber(curr_word_, line_number_);
    } catch (const std::out_of_range&) {
        throw CompileError::ImmediateOutOfRange(curr_word_, line_number_);
    }
}

args::Address Compiler::Impl::GetAddress(bool is_entry) {
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

        if (is_entry) {
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

args::RMArgs Compiler::Impl::GetRMArgs() {
    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::RMCommandNoRegister(line_number_);
    }

    args::Register reg = GetRegister();

    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::RMCommandNoAddress(line_number_);
    }

    return {reg, GetAddress()};
}

args::RRArgs Compiler::Impl::GetRRArgs() {
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

args::RIArgs Compiler::Impl::GetRIArgs() {
    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::RICommandNoRegister(line_number_);
    }

    args::Register reg = GetRegister();

    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::RICommandNoImmediate(line_number_);
    }

    return {reg, GetImmediate(args::kImmSize)};
}

args::JArgs Compiler::Impl::GetJArgs() {
    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::JCommandNoAddress(line_number_);
    }

    return GetAddress();
}

void Compiler::Impl::ProcessCurrLine() {
    if (!(curr_line_ >> curr_word_)) {
        return;
    }

    if (TryProcessEntrypoint()) {
        return;
    }

    if (TryProcessLabel() && !(curr_line_ >> curr_word_)) {
        return;
    }

    auto [code, format] = GetCommand();
    switch (format) {
        case cmd::RM: {
            cmd::Bin bin = cmd::build::RM(code, GetRMArgs());
            compiled_.push_back(bin);
            break;
        }

        case cmd::RR: {
            cmd::Bin bin = cmd::build::RR(code, GetRRArgs());
            compiled_.push_back(bin);
            break;
        }

        case cmd::RI: {
            cmd::Bin bin = cmd::build::RI(code, GetRIArgs());
            compiled_.push_back(bin);
            break;
        }

        case cmd::J: {
            cmd::Bin bin = cmd::build::J(code, GetJArgs());
            compiled_.push_back(bin);
            break;
        }

        default: {
            throw InternalError::UnknownCommandFormat(format, line_number_);
        }
    }

    if (curr_line_ >> curr_word_) {
        CompileError::ExtraWords(format, curr_word_, line_number_);
    }

    ++command_number_;
}

////////////////////////////////////////////////////////////////////////////////
///                            Labels substitution                           ///
////////////////////////////////////////////////////////////////////////////////

void Compiler::Impl::FillLabels() {
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
///                                Prepare data                              ///
////////////////////////////////////////////////////////////////////////////////

void Compiler::Impl::PrepareExecData(std::istream& code) {
    for (std::string line; std::getline(code, line); ++line_number_) {
        // ignore comments
        line.resize(std::min(line.find(syntax::kCommentSep), line.size()));

        curr_line_ = std::istringstream(line);
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

////////////////////////////////////////////////////////////////////////////////
///                           Compile from istream                           ///
////////////////////////////////////////////////////////////////////////////////

void Compiler::Impl::CompileImpl(std::istream& code,
                                 const std::string& exec_path) {
    PrepareExecData(code);

    exec::Data data{
        .entrypoint    = entrypoint_,
        .initial_stack = static_cast<types::Word>(arch::kMemorySize - 1),
        .code          = compiled_,
        // TODO: constants and data
        .constants = std::vector<arch::Word>(),
        .data      = std::vector<arch::Word>(),
    };

    exec::Write(data, exec_path);
}

void Compiler::Impl::MustCompile(std::istream& code,
                                 const std::string& exec_path) {
    CompileImpl(code, exec_path);
}

void Compiler::Impl::Compile(std::istream& code, const std::string& exec_path) {
    try {
        CompileImpl(code, exec_path);
    } catch (const Error& e) {
        std::cout << e.what() << std::endl;
    } catch (const exec::Error& e) {
        std::cout << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "compiler: unexpected exception: " << e.what()
                  << std::endl;
    } catch (...) {
        std::cout << "compiler: unexpected exception" << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////
///                             Compile from file                            ///
////////////////////////////////////////////////////////////////////////////////

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void Compiler::Impl::CompileImpl(const std::string& src,
                                 const std::string& dst) {
    std::ifstream code(src);
    if (code.fail()) {
        throw InternalError::FailedToOpen(src);
    }

    std::string real_dst = dst;
    if (real_dst.empty()) {
        std::filesystem::path src_path(src);

        std::filesystem::path dst_path = src_path.parent_path();
        dst_path /= src_path.stem().replace_extension(exec::kDefaultExtension);

        real_dst = dst_path.string();
    }

    CompileImpl(code, real_dst);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void Compiler::Impl::MustCompile(const std::string& src,
                                 const std::string& dst) {
    CompileImpl(src, dst);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void Compiler::Impl::Compile(const std::string& src, const std::string& dst) {
    try {
        CompileImpl(src, dst);
    } catch (const Error& e) {
        std::cout << e.what() << std::endl;
    } catch (const exec::Error& e) {
        std::cout << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "compiler: unexpected exception: " << e.what()
                  << std::endl;
    } catch (...) {
        std::cout << "compiler: unexpected exception" << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////
///                             Exported wrappers                            ///
////////////////////////////////////////////////////////////////////////////////

void Compiler::MustCompile(std::istream& code, const std::string& exec_path) {
    Impl impl;
    impl.MustCompile(code, exec_path);
}

void Compiler::Compile(std::istream& code, const std::string& exec_path) {
    Impl impl;
    impl.Compile(code, exec_path);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void Compiler::MustCompile(const std::string& src, const std::string& dst) {
    Impl impl;
    impl.MustCompile(src, dst);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void Compiler::Compile(const std::string& src, const std::string& dst) {
    Impl impl;
    impl.Compile(src, dst);
}

}  // namespace karma
