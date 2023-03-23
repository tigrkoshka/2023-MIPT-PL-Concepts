#include "file_compiler.hpp"

#include <bit>          // for bit_cast
#include <cstddef>      // for size_t
#include <cstdint>      // for int32_t
#include <stdexcept>    // for invalid_argument, out_of_range
#include <string>       // for string, stoull, stod
#include <type_traits>  // for make_unsigned_t
#include <utility>      // for exchange

#include "compiler/entrypoint.hpp"
#include "compiler/file.hpp"
#include "compiler/file_data.hpp"
#include "compiler/labels.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/constants.hpp"
#include "specs/syntax.hpp"
#include "utils/strings.hpp"
#include "utils/types.hpp"

namespace karma {

namespace utils  = detail::utils;
namespace syntax = detail::specs::syntax;
namespace cmd    = detail::specs::cmd;
namespace args   = cmd::args;
namespace consts = detail::specs::consts;
namespace arch   = detail::specs::arch;

////////////////////////////////////////////////////////////////////////////////
///                                   Utils                                  ///
////////////////////////////////////////////////////////////////////////////////

std::string Compiler::FileCompiler::Where() const {
    return file_->Where();
}

void Compiler::FileCompiler::SkipIncludes() {
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

bool Compiler::FileCompiler::TryProcessLabel() {
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

    latest_label_          = curr_token_;
    latest_label_pos_      = Where();
    latest_word_was_label_ = true;

    return true;
}

bool Compiler::FileCompiler::TryProcessEntrypoint() {
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

    if (!file_->GetToken(curr_token_)) {
        throw CompileError::EntrypointWithoutAddress(Where());
    }

    data_.entrypoint.Record(GetAddress(true), Where());

    if (file_->GetToken(curr_token_)) {
        throw CompileError::ExtraAfterEntrypoint({curr_token_, Where()});
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
///                        Constants value processing                        ///
////////////////////////////////////////////////////////////////////////////////

void Compiler::FileCompiler::ProcessUint32Constant() {
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

        data_.constants.push_back(value);
    } catch (...) {
        throw CompileError::InvalidConstValue(consts::UINT32,
                                              {curr_token_, Where()});
    }
}

void Compiler::FileCompiler::ProcessUint64Constant() {
    try {
        size_t pos{};

        // there is no function specifically for uint32_t values
        consts::UInt64 value = std::stoull(curr_token_, &pos, 0);

        if (pos != curr_token_.size()) {
            throw CompileError::InvalidConstValue(consts::UINT64,
                                                  {curr_token_, Where()});
        }

        auto [low, high] = utils::types::Split(value);
        data_.constants.push_back(low);
        data_.constants.push_back(high);
    } catch (...) {
        throw CompileError::InvalidConstValue(consts::UINT64,
                                              {curr_token_, Where()});
    }
}

void Compiler::FileCompiler::ProcessDoubleConstant() {
    try {
        size_t pos{};

        consts::Double value = std::stod(curr_token_, &pos);

        if (pos != curr_token_.size()) {
            throw CompileError::InvalidConstValue(consts::DOUBLE,
                                                  {curr_token_, Where()});
        }

        auto value_as_uint64 = std::bit_cast<consts::UInt64>(value);

        auto [low, high] = utils::types::Split(value_as_uint64);
        data_.constants.push_back(low);
        data_.constants.push_back(high);
    } catch (...) {
        throw CompileError::InvalidConstValue(consts::DOUBLE,
                                              {curr_token_, Where()});
    }
}

void Compiler::FileCompiler::ProcessCharConstant() {
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

    data_.constants.push_back(static_cast<arch::Word>(curr_token_[0]));
}

void Compiler::FileCompiler::ProcessStringConstant() {
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
        data_.constants.push_back(static_cast<arch::Word>(symbol));
    }
    data_.constants.push_back(static_cast<arch::Word>(consts::kStringEnd));
}

////////////////////////////////////////////////////////////////////////////////
///                           Constants processing                           ///
////////////////////////////////////////////////////////////////////////////////

bool Compiler::FileCompiler::TryProcessConstant() {
    if (curr_token_.empty()) {
        throw InternalError::EmptyWord(Where());
    }

    if (!consts::kNameToType.contains(curr_token_)) {
        return false;
    }

    if (std::exchange(latest_word_was_label_, false)) {
        // add 1 to the start of the current constant record, because
        // the memory representation of any constant starts with one-word
        // prefix specifying the constant's type for disassembling
        data_.labels.RecordConstantLabel(latest_label_,
                                         data_.constants.size() + 1,
                                         latest_label_pos_);
    }

    consts::Type type = consts::kNameToType.at(curr_token_);

    if (!file_->GetLine(curr_token_)) {
        throw CompileError::EmptyConstValue(type, Where());
    }

    data_.constants.push_back(type);

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

cmd::CodeFormat Compiler::FileCompiler::GetCodeFormat() const {
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

args::Register Compiler::FileCompiler::GetRegister() const {
    if (curr_token_.empty()) {
        throw InternalError::EmptyWord(Where());
    }

    if (!arch::kRegisterNameToNum.contains(curr_token_)) {
        throw CompileError::UnknownRegister({curr_token_, Where()});
    }

    return arch::kRegisterNameToNum.at(curr_token_);
}

args::Immediate Compiler::FileCompiler::GetImmediate(size_t bit_size) const {
    if (curr_token_.empty()) {
        throw InternalError::EmptyWord(Where());
    }

    using Int  = args::Immediate;
    using Uint = std::make_unsigned_t<Int>;

    const auto min_modulo = static_cast<Uint>(1ull)
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

args::Address Compiler::FileCompiler::GetAddress(bool is_entrypoint) {
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
            data_.labels.RecordEntrypointLabel(curr_token_);
        } else {
            data_.labels.RecordUsage(curr_token_, data_.code.size(), Where());
        }

        return 0;
    } catch (const std::out_of_range&) {
        throw CompileError::AddressOutOfMemory({curr_token_, Where()});
    }
}

////////////////////////////////////////////////////////////////////////////////
///                           Command args parsing                           ///
////////////////////////////////////////////////////////////////////////////////

args::RMArgs Compiler::FileCompiler::GetRMArgs() {
    if (!file_->GetToken(curr_token_)) {
        throw CompileError::RMNoRegister(Where());
    }

    args::Register reg = GetRegister();

    if (!file_->GetToken(curr_token_)) {
        throw CompileError::RMNoAddress(Where());
    }

    return {.reg = reg, .addr = GetAddress()};
}

args::RRArgs Compiler::FileCompiler::GetRRArgs() {
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

args::RIArgs Compiler::FileCompiler::GetRIArgs() {
    if (!file_->GetToken(curr_token_)) {
        throw CompileError::RINoRegister(Where());
    }

    args::Register reg = GetRegister();

    if (!file_->GetToken(curr_token_)) {
        throw CompileError::RINoImmediate(Where());
    }

    return {.reg = reg, .imm = GetImmediate(args::kImmSize)};
}

args::JArgs Compiler::FileCompiler::GetJArgs() {
    if (!file_->GetToken(curr_token_)) {
        throw CompileError::JNoAddress(Where());
    }

    return {.addr = GetAddress()};
}

////////////////////////////////////////////////////////////////////////////////
///                              Command parsing                             ///
////////////////////////////////////////////////////////////////////////////////

cmd::Bin Compiler::FileCompiler::MustParseCommand() {
    cmd::Bin bin{};

    auto [code, format] = GetCodeFormat();

    if (std::exchange(latest_word_was_label_, false)) {
        data_.labels.RecordCommandLabel(latest_label_,
                                        data_.code.size(),
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

void Compiler::FileCompiler::ProcessCurrLine(bool is_first_line) {
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

    data_.code.push_back(MustParseCommand());
}

////////////////////////////////////////////////////////////////////////////////
///                              File compiling                              ///
////////////////////////////////////////////////////////////////////////////////

Compiler::FileData Compiler::FileCompiler::PrepareData() && {
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

}  // namespace karma
