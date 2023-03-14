#include "file_compiler.hpp"

#include <cstddef>      // for size_t
#include <cstdint>      // for int32_t
#include <optional>     // for optional
#include <stdexcept>    // for invalid_argument, out_of_range
#include <string>       // for string, stoull, stod
#include <type_traits>  // for make_unsigned_t
#include <utility>      // for exchange

#include "errors.hpp"
#include "exec_data.hpp"
#include "labels.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/constants.hpp"
#include "specs/syntax.hpp"
#include "utils/strings.hpp"
#include "utils/type_conversions.hpp"

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

////////////////////////////////////////////////////////////////////////////////
///                                   Utils                                  ///
////////////////////////////////////////////////////////////////////////////////

std::string FileCompiler::Where() const {
    return file_->Where();
}

FileCompiler::Segment& FileCompiler::Code() {
    return data_.Code();
}

const FileCompiler::Segment& FileCompiler::Code() const {
    return data_.Code();
}

FileCompiler::Segment& FileCompiler::Constants() {
    return data_.Constants();
}

const FileCompiler::Segment& FileCompiler::Constants() const {
    return data_.Constants();
}

size_t FileCompiler::CurrCmdAddress() const {
    return Code().size();
}

size_t FileCompiler::CurrConstAddress() const {
    // add 1 to the start of the current constant record, because
    // the memory representation of any constant starts with one-word
    // prefix specifying the constant's type for disassembling
    return Constants().size() + 1;
}

void FileCompiler::SkipIncludes() {
    while (file_->NextLine()) {
        if (!file_->GetToken(curr_token_)) {
            continue;
        }

        if (curr_token_ != syntax::kIncludeDirective) {
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
///                            Line types parsing                            ///
////////////////////////////////////////////////////////////////////////////////

bool FileCompiler::TryProcessLabel() {
    if (curr_token_.empty()) {
        throw InternalError::EmptyWord(Where());
    }

    if (curr_token_.back() != syntax::kLabelEnd) {
        return false;
    }

    curr_token_.resize(curr_token_.size() - 1);

    if (std::exchange(latest_word_was_label_, true)) {
        throw CompileError::ConsecutiveLabels(
            {curr_token_, Where()},
            {latest_label_, latest_label_pos_});
    }

    Labels::CheckLabel(curr_token_, Where());

    if (std::optional<std::string> prev = labels_->TryGetPos(curr_token_)) {
        throw CompileError::LabelRedefinition({curr_token_, Where()}, *prev);
    }

    latest_label_          = curr_token_;
    latest_label_pos_      = Where();
    latest_word_was_label_ = true;

    return true;
}

bool FileCompiler::TryProcessEntrypoint() {
    if (curr_token_.empty()) {
        throw InternalError::EmptyWord(Where());
    }

    if (curr_token_ != syntax::kEntrypointDirective) {
        return false;
    }

    if (std::exchange(latest_word_was_label_, false)) {
        throw CompileError::LabelBeforeEntrypoint(
            Where(),
            {latest_label_, latest_label_pos_});
    }

    if (std::optional<std::string> prev = entrypoint_->TryGetPos()) {
        throw CompileError::SecondEntrypoint(Where(), *prev);
    }

    if (!file_->GetToken(curr_token_)) {
        throw CompileError::EntrypointWithoutAddress(Where());
    }

    entrypoint_->Record(GetAddress(true), Where());

    if (file_->GetToken(curr_token_)) {
        throw CompileError::ExtraAfterEntrypoint({curr_token_, Where()});
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
///                        Constants value processing                        ///
////////////////////////////////////////////////////////////////////////////////

void FileCompiler::ProcessUint32Constant() {
    try {
        size_t pos{};

        // there is no function specifically for uint32_t values
        //
        // not that if a user specified a negative value, after the static cast
        // we will have interpreted it correctly, because taking a number
        // modulo 2^32 is still correct if we have taken it modulo 2^64 first
        auto value =
            static_cast<consts::UInt32>(std::stoull(curr_token_, &pos, 0));

        if (pos != curr_token_.size()) {
            throw CompileError::InvalidConstValue(consts::UINT32,
                                                  {curr_token_, Where()});
        }

        Constants().push_back(value);
    } catch (...) {
        throw CompileError::InvalidConstValue(consts::UINT32,
                                              {curr_token_, Where()});
    }
}

void FileCompiler::ProcessUint64Constant() {
    try {
        size_t pos{};

        // there is no function specifically for uint32_t values
        consts::UInt64 value = std::stoull(curr_token_, &pos, 0);

        if (pos != curr_token_.size()) {
            throw CompileError::InvalidConstValue(consts::UINT64,
                                                  {curr_token_, Where()});
        }

        auto [low, high] = utils::types::Split(value);
        Constants().push_back(low);
        Constants().push_back(high);
    } catch (...) {
        throw CompileError::InvalidConstValue(consts::UINT64,
                                              {curr_token_, Where()});
    }
}

void FileCompiler::ProcessDoubleConstant() {
    try {
        size_t pos{};

        consts::Double value = std::stod(curr_token_, &pos);

        if (pos != curr_token_.size()) {
            throw CompileError::InvalidConstValue(consts::DOUBLE,
                                                  {curr_token_, Where()});
        }

        auto [low, high] = utils::types::Split(utils::types::ToUll(value));
        Constants().push_back(low);
        Constants().push_back(high);
    } catch (...) {
        throw CompileError::InvalidConstValue(consts::DOUBLE,
                                              {curr_token_, Where()});
    }
}

void FileCompiler::ProcessCharConstant() {
    if (curr_token_.size() < 2) {
        throw CompileError::CharTooSmallForQuotes({curr_token_, Where()});
    }

    if (curr_token_.front() != consts::kCharQuote) {
        throw CompileError::CharNoStartQuote({curr_token_, Where()});
    }

    if (curr_token_.back() != consts::kCharQuote) {
        throw CompileError::CharNoEndQuote({curr_token_, Where()});
    }

    curr_token_ = curr_token_.substr(1, curr_token_.size() - 2);

    utils::strings::Unescape(curr_token_);

    if (curr_token_.size() != 1) {
        throw CompileError::InvalidConstValue(consts::CHAR,
                                              {curr_token_, Where()});
    }

    Constants().push_back(static_cast<arch::Word>(curr_token_[0]));
}

void FileCompiler::ProcessStringConstant() {
    if (curr_token_.size() < 2) {
        throw CompileError::StringTooSmallForQuotes({curr_token_, Where()});
    }

    if (curr_token_.front() != consts::kStringQuote) {
        throw CompileError::StringNoStartQuote({curr_token_, Where()});
    }

    if (curr_token_.back() != consts::kStringQuote) {
        throw CompileError::StringNoEndQuote({curr_token_, Where()});
    }

    curr_token_ = curr_token_.substr(1, curr_token_.size() - 2);

    utils::strings::Unescape(curr_token_);

    for (consts::Char symbol : curr_token_) {
        Constants().push_back(static_cast<arch::Word>(symbol));
    }
    Constants().push_back(static_cast<arch::Word>(consts::kStringEnd));
}

////////////////////////////////////////////////////////////////////////////////
///                           Constants processing                           ///
////////////////////////////////////////////////////////////////////////////////

bool FileCompiler::TryProcessConstant() {
    if (curr_token_.empty()) {
        throw InternalError::EmptyWord(Where());
    }

    if (!consts::kNameToType.contains(curr_token_)) {
        return false;
    }

    if (std::exchange(latest_word_was_label_, false)) {
        labels_->RecordConstantLabel(latest_label_,
                                     CurrConstAddress(),
                                     latest_label_pos_);
    }

    consts::Type type = consts::kNameToType.at(curr_token_);

    if (!file_->GetLine(curr_token_)) {
        throw CompileError::EmptyConstValue(type, Where());
    }

    Constants().push_back(type);

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
            throw InternalError::UnprocessedConstantType(type, Where());
        }
    }

    if (file_->GetToken(curr_token_)) {
        throw CompileError::ExtraAfterConstant(type, {curr_token_, Where()});
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
///                           Command word parsing                           ///
////////////////////////////////////////////////////////////////////////////////

cmd::CodeFormat FileCompiler::GetCodeFormat() const {
    if (!cmd::kNameToCode.contains(curr_token_)) {
        throw CompileError::UnknownCommand({curr_token_, Where()});
    }

    cmd::Code code = cmd::kNameToCode.at(curr_token_);

    if (!cmd::kCodeToFormat.contains(code)) {
        throw InternalError::FormatNotFound(code, Where());
    }

    cmd::Format format = cmd::kCodeToFormat.at(code);

    return {code, format};
}

args::Register FileCompiler::GetRegister() const {
    if (curr_token_.empty()) {
        throw InternalError::EmptyWord(Where());
    }

    if (!arch::kRegisterNameToNum.contains(curr_token_)) {
        throw CompileError::UnknownRegister({curr_token_, Where()});
    }

    return arch::kRegisterNameToNum.at(curr_token_);
}

args::Immediate FileCompiler::GetImmediate(size_t bit_size) const {
    if (curr_token_.empty()) {
        throw InternalError::EmptyWord(Where());
    }

    using Int  = args::Immediate;
    using Uint = std::make_unsigned_t<Int>;

    const auto min_modulo = static_cast<Uint>(1)
                            << static_cast<Uint>(bit_size - 1ull);

    const auto min = -static_cast<Int>(min_modulo);
    const auto max = static_cast<Int>(min_modulo - static_cast<Uint>(1));

    try {
        size_t pos{};
        Int operand = std::stoi(curr_token_, &pos, 0);

        if (pos != curr_token_.size()) {
            throw CompileError::ImmediateNotANumber({curr_token_, Where()});
        }

        if (operand < min) {
            throw CompileError::ImmediateLessThanMin(min,
                                                     {curr_token_, Where()});
        }

        if (operand > max) {
            throw CompileError::ImmediateMoreThanMax(max,
                                                     {curr_token_, Where()});
        }

        return operand;
    } catch (const std::invalid_argument&) {
        throw CompileError::ImmediateNotANumber({curr_token_, Where()});
    } catch (const std::out_of_range&) {
        throw CompileError::ImmediateOutOfRange({curr_token_, Where()});
    }
}

args::Address FileCompiler::GetAddress(bool is_entrypoint) {
    if (curr_token_.empty()) {
        throw InternalError::EmptyWord(Where());
    }

    try {
        size_t pos{};

        // we do not use std::stoll, because a valid address must be a 20-bit
        // unsigned value, so if it does not fit into a 32-bit signed integer,
        // it is out of range
        //
        // we do not use std::stoul, because we want to give a separate
        // compile error if a user specified a negative value as an address
        int32_t operand = std::stoi(curr_token_, &pos, 0);

        if (pos != curr_token_.size()) {
            // this means that there is a number in the beginning of word,
            // but the whole word is not a number
            //
            // assume the programmer meant it as a label,
            // but a label cannot start with a digit
            //
            // NOTE: we do allow '0', '0x' and '0X' prefixed numbers,
            //       but in all those cases the word still starts with
            //       a digit ('0') and thus is not a valid label
            throw CompileError::LabelStartsWithDigit({curr_token_, Where()});
        }

        if (operand < 0) {
            throw CompileError::AddressNegative({curr_token_, Where()});
        }

        auto addr = static_cast<args::Address>(operand);
        if (addr > arch::kMemorySize) {
            throw CompileError::AddressOutOfMemory({curr_token_, Where()});
        }

        return addr;
    } catch (const std::invalid_argument&) {
        // this implies that the word does not start with a digit,
        // so we assume it's a label
        Labels::CheckLabel(curr_token_, Where());

        if (is_entrypoint) {
            labels_->RecordEntrypointLabel(curr_token_);
        } else {
            labels_->RecordUsage(curr_token_, file_.get(), CurrCmdAddress());
        }

        return 0;
    } catch (const std::out_of_range&) {
        throw CompileError::AddressOutOfMemory({curr_token_, Where()});
    }
}

////////////////////////////////////////////////////////////////////////////////
///                           Command args parsing                           ///
////////////////////////////////////////////////////////////////////////////////

args::RMArgs FileCompiler::GetRMArgs() {
    if (!file_->GetToken(curr_token_)) {
        throw CompileError::RMNoRegister(Where());
    }

    args::Register reg = GetRegister();

    if (!file_->GetToken(curr_token_)) {
        throw CompileError::RMNoAddress(Where());
    }

    return {.reg = reg, .addr = GetAddress()};
}

args::RRArgs FileCompiler::GetRRArgs() {
    if (!file_->GetToken(curr_token_)) {
        throw CompileError::RRNoReceiver(Where());
    }

    args::Register recv = GetRegister();

    if (!file_->GetToken(curr_token_)) {
        throw CompileError::RRNoSource(Where());
    }

    args::Register src = GetRegister();

    if (!file_->GetToken(curr_token_)) {
        throw CompileError::RRNoModifier(Where());
    }

    return {.recv = recv, .src = src, .mod = GetImmediate(args::kModSize)};
}

args::RIArgs FileCompiler::GetRIArgs() {
    if (!file_->GetToken(curr_token_)) {
        throw CompileError::RINoRegister(Where());
    }

    args::Register reg = GetRegister();

    if (!file_->GetToken(curr_token_)) {
        throw CompileError::RINoImmediate(Where());
    }

    return {.reg = reg, .imm = GetImmediate(args::kImmSize)};
}

args::JArgs FileCompiler::GetJArgs() {
    if (!file_->GetToken(curr_token_)) {
        throw CompileError::JNoAddress(Where());
    }

    return {.addr = GetAddress()};
}

////////////////////////////////////////////////////////////////////////////////
///                              Command parsing                             ///
////////////////////////////////////////////////////////////////////////////////

cmd::Bin FileCompiler::MustParseCommand() {
    cmd::Bin bin{};

    auto [code, format] = GetCodeFormat();

    if (std::exchange(latest_word_was_label_, false)) {
        labels_->RecordCommandLabel(latest_label_,
                                    CurrCmdAddress(),
                                    latest_label_pos_);
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
            throw InternalError::UnprocessedCommandFormat(format, Where());
        }
    }

    if (file_->GetToken(curr_token_)) {
        throw CompileError::ExtraAfterCommand(format, {curr_token_, Where()});
    }

    return bin;
}

////////////////////////////////////////////////////////////////////////////////
///                               Line parsing                               ///
////////////////////////////////////////////////////////////////////////////////

void FileCompiler::ProcessCurrLine(bool is_first_line) {
    if (!is_first_line && !file_->GetToken(curr_token_)) {
        return;
    }

    if (TryProcessLabel() && !file_->GetToken(curr_token_)) {
        return;
    }

    if (TryProcessEntrypoint()) {
        return;
    }

    if (TryProcessConstant()) {
        return;
    }

    Code().push_back(MustParseCommand());
}

////////////////////////////////////////////////////////////////////////////////
///                              File compiling                              ///
////////////////////////////////////////////////////////////////////////////////

ExecData FileCompiler::PrepareData() && {
    file_->Open();

    SkipIncludes();

    // process the line for which the first token
    // was already acquired in the line above
    ProcessCurrLine(true);

    // process the rest of the file
    while (file_->NextLine()) {
        ProcessCurrLine();
    };

    if (latest_word_was_label_) {
        throw CompileError::FileEndsWithLabel(
            {latest_label_, latest_label_pos_});
    }

    file_->Close();

    return std::move(data_);
}

}  // namespace karma::compiler::detail
