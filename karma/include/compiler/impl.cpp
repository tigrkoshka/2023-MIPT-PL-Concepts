#include "impl.hpp"

#include <algorithm>    // for min
#include <cstddef>      // for size_t
#include <exception>    // for exception
#include <filesystem>   // for path
#include <fstream>      // for ifstream
#include <iostream>     // for cout, ios
#include <stdexcept>    // for invalid_argument, out_of_range
#include <string>       // for string, stoull, stod
#include <type_traits>  // for make_unsigned_t
#include <utility>      // for move

#include "exec/exec.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/constants.hpp"
#include "specs/exec.hpp"
#include "specs/syntax.hpp"
#include "utils/strings.hpp"
#include "utils/types.hpp"

#include "errors.hpp"

namespace karma::compiler::detail {

using errors::compiler::Error;
using errors::compiler::InternalError;
using errors::compiler::CompileError;

namespace utils = karma::detail::utils;

namespace syntax = karma::detail::specs::syntax;

namespace cmd  = karma::detail::specs::cmd;
namespace args = cmd::args;

namespace consts = karma::detail::specs::consts;

namespace arch = karma::detail::specs::arch;

namespace exec = karma::detail::exec;

////////////////////////////////////////////////////////////////////////////////
///                                   Utils                                  ///
////////////////////////////////////////////////////////////////////////////////

size_t Impl::CurrCmdAddress() {
    return code_.size();
}

size_t Impl::CurrConstAddress() {
    // add 1 to the start of the current constant record, because
    // the memory representation of any constant starts with one-word
    // prefix specifying the constant's type for disassembling
    return constants_.size() + 1;
}

////////////////////////////////////////////////////////////////////////////////
///                            Line types parsing                            ///
////////////////////////////////////////////////////////////////////////////////

bool Impl::TryProcessLabel() {
    if (curr_word_.empty()) {
        throw InternalError::EmptyWord(line_number_);
    }

    if (curr_word_.back() != syntax::kLabelEnd) {
        return false;
    }

    curr_word_.resize(curr_word_.size() - 1);

    if (std::exchange(latest_word_was_label_, true)) {
        throw CompileError::ConsecutiveLabels(curr_word_,
                                              line_number_,
                                              labels_.GetLatest(),
                                              labels_.GetLatestLine());
    }

    Labels::CheckLabel(curr_word_, line_number_);

    if (std::optional<size_t> previous_definition_line =
            labels_.TryGetDefinitionLine(curr_word_)) {
        throw CompileError::LabelRedefinition(curr_word_,
                                              line_number_,
                                              *previous_definition_line);
    }

    labels_.RecordLabelOccurrence(curr_word_, line_number_);

    return true;
}

bool Impl::TryProcessEntrypoint() {
    if (curr_word_.empty()) {
        throw InternalError::EmptyWord(line_number_);
    }

    if (curr_word_ != syntax::kEntrypointDirective) {
        return false;
    }

    if (std::exchange(latest_word_was_label_, false)) {
        throw CompileError::LabelBeforeEntrypoint(line_number_,
                                                  labels_.GetLatest(),
                                                  labels_.GetLatestLine());
    }

    if (seen_entrypoint_) {
        throw CompileError::SecondEntrypoint(line_number_, entrypoint_line_);
    }

    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::EntrypointWithoutAddress(line_number_);
    }

    entrypoint_      = GetAddress(true);
    entrypoint_line_ = line_number_;
    seen_entrypoint_ = true;

    if (curr_line_ >> curr_word_) {
        throw CompileError::ExtraWordsAfterEntrypoint(curr_word_, line_number_);
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
///                        Constants value processing                        ///
////////////////////////////////////////////////////////////////////////////////

void Impl::ProcessUint32Constant() {
    try {
        size_t pos{};

        // there is no function specifically for uint32_t values
        //
        // not that if a user specified a negative value, after the static cast
        // we will have interpreted it correctly, because taking a number
        // modulo 2^32 is still correct if we have taken it modulo 2^64 first
        auto value =
            static_cast<consts::UInt32>(std::stoull(curr_word_, &pos, 0));

        if (pos != curr_word_.size()) {
            throw CompileError::InvalidValue(consts::UINT32,
                                             curr_word_,
                                             line_number_);
        }

        constants_.push_back(value);
    } catch (...) {
        throw CompileError::InvalidValue(consts::UINT32,
                                         curr_word_,
                                         line_number_);
    }
}

void Impl::ProcessUint64Constant() {
    try {
        size_t pos{};

        // there is no function specifically for uint32_t values
        consts::UInt64 value = std::stoull(curr_word_, &pos, 0);

        if (pos != curr_word_.size()) {
            throw CompileError::InvalidValue(consts::UINT64,
                                             curr_word_,
                                             line_number_);
        }

        auto [low, high] = utils::types::Split(value);
        constants_.push_back(low);
        constants_.push_back(high);
    } catch (...) {
        throw CompileError::InvalidValue(consts::UINT64,
                                         curr_word_,
                                         line_number_);
    }
}

void Impl::ProcessDoubleConstant() {
    try {
        size_t pos{};

        consts::Double value = std::stod(curr_word_, &pos);

        if (pos != curr_word_.size()) {
            throw CompileError::InvalidValue(consts::DOUBLE,
                                             curr_word_,
                                             line_number_);
        }

        auto [low, high] = utils::types::Split(utils::types::ToUll(value));
        constants_.push_back(low);
        constants_.push_back(high);
    } catch (...) {
        throw CompileError::InvalidValue(consts::DOUBLE,
                                         curr_word_,
                                         line_number_);
    }
}

void Impl::ProcessCharConstant() {
    if (curr_word_.size() < 2) {
        throw CompileError::CharTooSmallForQuotes(curr_word_, line_number_);
    }

    if (curr_word_.front() != consts::kCharQuote) {
        throw CompileError::CharNoStartQuote(curr_word_, line_number_);
    }

    if (curr_word_.back() != consts::kCharQuote) {
        throw CompileError::CharNoEndQuote(curr_word_, line_number_);
    }

    curr_word_ = curr_word_.substr(1, curr_word_.size() - 2);

    utils::strings::Unescape(curr_word_);

    if (curr_word_.size() != 1) {
        throw CompileError::InvalidValue(consts::CHAR,
                                         curr_word_,
                                         line_number_);
    }

    constants_.push_back(static_cast<arch::Word>(curr_word_[0]));
}

void Impl::ProcessStringConstant() {
    if (curr_word_.size() < 2) {
        throw CompileError::StringTooSmallForQuotes(curr_word_, line_number_);
    }

    if (curr_word_.front() != consts::kStringQuote) {
        throw CompileError::StringNoStartQuote(curr_word_, line_number_);
    }

    if (curr_word_.back() != consts::kStringQuote) {
        throw CompileError::StringNoEndQuote(curr_word_, line_number_);
    }

    curr_word_ = curr_word_.substr(1, curr_word_.size() - 2);

    utils::strings::Unescape(curr_word_);

    for (consts::Char symbol : curr_word_) {
        constants_.push_back(static_cast<arch::Word>(symbol));
    }
    constants_.push_back(static_cast<arch::Word>(consts::kStringEnd));
}

////////////////////////////////////////////////////////////////////////////////
///                           Constants processing                           ///
////////////////////////////////////////////////////////////////////////////////

bool Impl::TryProcessConstant() {
    if (curr_word_.empty()) {
        throw InternalError::EmptyWord(line_number_);
    }

    if (!consts::kNameToType.contains(curr_word_)) {
        return false;
    }

    if (std::exchange(latest_word_was_label_, false)) {
        labels_.RecordConstantLabel(CurrConstAddress());
    }

    consts::Type type = consts::kNameToType.at(curr_word_);

    getline(curr_line_, curr_word_);
    utils::strings::TrimSpaces(curr_word_);

    if (curr_word_.empty()) {
        throw CompileError::EmptyConstantValue(type, line_number_);
    }

    constants_.push_back(type);

    switch (type) {
        case consts::UINT32: {
            ProcessUint32Constant();
            break;
        }

        case consts::UINT64: {
            ProcessUint64Constant();
            break;
        }

        case consts::DOUBLE: {
            ProcessDoubleConstant();
            break;
        }

        case consts::CHAR: {
            ProcessCharConstant();
            break;
        }

        case consts::STRING: {
            ProcessStringConstant();
            break;
        }

        default: {
            throw InternalError::UnprocessedConstantType(type, line_number_);
        }
    }

    if (curr_line_ >> curr_word_) {
        throw CompileError::ExtraWordsAfterConstant(type,
                                                    curr_word_,
                                                    line_number_);
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
///                           Command word parsing                           ///
////////////////////////////////////////////////////////////////////////////////

cmd::CodeFormat Impl::GetCodeFormat() const {
    if (!cmd::kNameToCode.contains(curr_word_)) {
        throw CompileError::UnknownCommand(curr_word_, line_number_);
    }

    cmd::Code code = cmd::kNameToCode.at(curr_word_);

    if (!cmd::kCodeToFormat.contains(code)) {
        throw InternalError::FormatNotFound(code, line_number_);
    }

    cmd::Format format = cmd::kCodeToFormat.at(code);

    return {code, format};
}

args::Register Impl::GetRegister() const {
    if (curr_word_.empty()) {
        throw InternalError::EmptyWord(line_number_);
    }

    if (!arch::kRegisterNameToNum.contains(curr_word_)) {
        throw CompileError::UnknownRegister(curr_word_, line_number_);
    }

    return arch::kRegisterNameToNum.at(curr_word_);
}

args::Immediate Impl::GetImmediate(size_t bit_size) const {
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

args::Address Impl::GetAddress(bool is_entrypoint) {
    if (curr_word_.empty()) {
        throw InternalError::EmptyWord(line_number_);
    }

    try {
        size_t pos{};

        // we do not use std::stoll, because a valid address must be a 20-bit
        // unsigned value, so if it does not fit into a 32-bit signed integer,
        // it is out of range
        //
        // we do not use std::stoul, because we want to give a separate
        // compile error if a user specified a negative value as an address
        int32_t operand = std::stoi(curr_word_, &pos, 0);

        if (pos != curr_word_.size()) {
            // this means that there is a number in the beginning of word,
            // but the whole word is not a number
            //
            // assume the programmer meant it as a label,
            // but a label cannot start with a digit
            //
            // NOTE: we do allow '0', '0x' and '0X' prefixed numbers,
            //       but in all those cases the word still starts with
            //       a digit ('0') and thus is not a valid label
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
        Labels::CheckLabel(curr_word_, line_number_);

        if (is_entrypoint) {
            labels_.RecordEntrypointLabel(curr_word_);
        } else {
            labels_.RecordUsage(curr_word_, line_number_, CurrCmdAddress());
        }

        return 0;
    } catch (const std::out_of_range&) {
        throw CompileError::AddressOutOfMemory(curr_word_, line_number_);
    }
}

////////////////////////////////////////////////////////////////////////////////
///                           Command args parsing                           ///
////////////////////////////////////////////////////////////////////////////////

args::RMArgs Impl::GetRMArgs() {
    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::RMCommandNoRegister(line_number_);
    }

    args::Register reg = GetRegister();

    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::RMCommandNoAddress(line_number_);
    }

    return {reg, GetAddress()};
}

args::RRArgs Impl::GetRRArgs() {
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

args::RIArgs Impl::GetRIArgs() {
    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::RICommandNoRegister(line_number_);
    }

    args::Register reg = GetRegister();

    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::RICommandNoImmediate(line_number_);
    }

    return {reg, GetImmediate(args::kImmSize)};
}

args::JArgs Impl::GetJArgs() {
    if (!(curr_line_ >> curr_word_)) {
        throw CompileError::JCommandNoAddress(line_number_);
    }

    return GetAddress();
}

////////////////////////////////////////////////////////////////////////////////
///                              Command parsing                             ///
////////////////////////////////////////////////////////////////////////////////

cmd::Bin Impl::MustParseCommand() {
    cmd::Bin bin{};

    auto [code, format] = GetCodeFormat();

    if (std::exchange(latest_word_was_label_, false)) {
        labels_.RecordCommandLabel(CurrCmdAddress());
    }

    switch (format) {
        case cmd::RM: {
            bin = cmd::build::RM(code, GetRMArgs());
            break;
        }

        case cmd::RR: {
            bin = cmd::build::RR(code, GetRRArgs());
            break;
        }

        case cmd::RI: {
            bin = cmd::build::RI(code, GetRIArgs());
            break;
        }

        case cmd::J: {
            bin = cmd::build::J(code, GetJArgs());
            break;
        }

        default: {
            throw InternalError::UnprocessedCommandFormat(format, line_number_);
        }
    }

    if (curr_line_ >> curr_word_) {
        throw CompileError::ExtraWordsAfterCommand(format,
                                                   curr_word_,
                                                   line_number_);
    }

    return bin;
}

////////////////////////////////////////////////////////////////////////////////
///                               Line parsing                               ///
////////////////////////////////////////////////////////////////////////////////

void Impl::ProcessCurrLine() {
    if (!(curr_line_ >> curr_word_)) {
        return;
    }

    if (TryProcessLabel() && !(curr_line_ >> curr_word_)) {
        return;
    }

    if (TryProcessEntrypoint()) {
        return;
    }

    if (TryProcessConstant()) {
        return;
    }

    code_.push_back(MustParseCommand());
}

////////////////////////////////////////////////////////////////////////////////
///                            Labels substitution                           ///
////////////////////////////////////////////////////////////////////////////////

void Impl::FillLabels() {
    for (const auto& [label, usages] : labels_.GetUsages()) {
        std::optional<size_t> definition_opt = labels_.TryGetDefinition(label);
        if (!definition_opt) {
            // use usages[0], because it is the first occurrence
            // of the label (we store label usages while parsing
            // the file line by line)
            throw CompileError::UndefinedLabel(label, usages[0].first);
        }

        auto definition = static_cast<args::Address>(*definition_opt);
        for (auto [_, n_cmd] : usages) {
            // the address always occupies the last
            // bits of the command binary
            code_[n_cmd] |= definition;
        }
    }

    const std::optional<std::string>& entrypoint_label_opt =
        labels_.TryGetEntrypointLabel();

    if (entrypoint_label_opt) {
        const std::string& entrypoint_label = *entrypoint_label_opt;

        std::optional<size_t> definition_opt =
            labels_.TryGetDefinition(entrypoint_label);
        if (!definition_opt) {
            // use usages[0], because it is the first occurrence
            // of the label (we store label usages while parsing
            // the file line by line)
            throw CompileError::UndefinedLabel(entrypoint_label,
                                               entrypoint_line_);
        }

        entrypoint_ = static_cast<args::Address>(*definition_opt);
    }
}

////////////////////////////////////////////////////////////////////////////////
///                                Prepare data                              ///
////////////////////////////////////////////////////////////////////////////////

size_t Impl::FindCommentStart(const std::string& line) {
    size_t curr_start_pos = 0;

    while (true) {
        size_t comment_start = line.find(syntax::kCommentSep, curr_start_pos);

        if (comment_start == curr_start_pos ||
            comment_start == std::string::npos) {
            return comment_start;
        }

        if (line[comment_start - 1] == '\\') {
            curr_start_pos = comment_start + 1;
            continue;
        }

        return comment_start;
    }
}

void Impl::PrepareExecData(std::istream& code) {
    for (std::string line; std::getline(code, line); ++line_number_) {
        // ignore comments
        line.resize(std::min(FindCommentStart(line), line.size()));

        curr_line_ = std::istringstream(line);
        ProcessCurrLine();
    }

    if (latest_word_was_label_) {
        throw CompileError::FileEndsWithLabel(labels_.GetLatest(),
                                              labels_.GetLatestLine());
    }

    if (!seen_entrypoint_) {
        throw CompileError::NoEntrypoint();
    }

    labels_.SetCodeSize(code_.size());

    FillLabels();
}

////////////////////////////////////////////////////////////////////////////////
///                           Compile from istream                           ///
////////////////////////////////////////////////////////////////////////////////

void Impl::CompileImpl(std::istream& code, const std::string& exec_path) {
    PrepareExecData(code);

    exec::Data data{
        .entrypoint    = entrypoint_,
        .initial_stack = static_cast<arch::Word>(arch::kMemorySize - 1),
        .code          = code_,
        .constants     = constants_,
    };

    exec::Write(data, exec_path);
}

void Impl::MustCompile(std::istream& code, const std::string& exec_path) {
    CompileImpl(code, exec_path);
}

void Impl::Compile(std::istream& code, const std::string& exec_path) {
    try {
        CompileImpl(code, exec_path);
    } catch (const Error& e) {
        std::cout << e.what() << std::endl;
    } catch (const karma::errors::Error& e) {
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

void Impl::CompileImpl(const std::string& src, const std::string& dst) {
    std::ifstream code(src);
    if (code.fail()) {
        throw InternalError::FailedToOpen(src);
    }

    std::string real_dst = dst;
    if (real_dst.empty()) {
        std::filesystem::path src_path(src);

        std::filesystem::path dst_path = src_path.parent_path();
        dst_path /= src_path.stem().replace_extension(
            karma::detail::specs::exec::kDefaultExtension);

        real_dst = dst_path.string();
    }

    CompileImpl(code, real_dst);
}

void Impl::MustCompile(const std::string& src, const std::string& dst) {
    CompileImpl(src, dst);
}

void Impl::Compile(const std::string& src, const std::string& dst) {
    try {
        CompileImpl(src, dst);
    } catch (const Error& e) {
        std::cout << e.what() << std::endl;
    } catch (const errors::Error& e) {
        std::cout << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "compiler: unexpected exception: " << e.what()
                  << std::endl;
    } catch (...) {
        std::cout << "compiler: unexpected exception" << std::endl;
    }
}

}  // namespace karma::compiler::detail
