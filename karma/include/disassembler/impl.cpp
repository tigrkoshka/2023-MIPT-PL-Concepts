#include "impl.hpp"

#include <bit>          // for bit_cast
#include <exception>    // for exception
#include <filesystem>   // for path
#include <fstream>      // for ofstream
#include <iomanip>      // for setprecision
#include <iostream>     // for cerr
#include <sstream>      // for ostringstream
#include <string>       // for string
#include <type_traits>  // for make_signed_t

#include "disassembler/labels.hpp"
#include "exec/exec.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/constants.hpp"
#include "specs/syntax.hpp"
#include "utils/error.hpp"
#include "utils/strings.hpp"
#include "utils/types.hpp"

namespace karma {

namespace utils  = detail::utils;
namespace arch   = detail::specs::arch;
namespace cmd    = detail::specs::cmd;
namespace args   = cmd::args;
namespace consts = detail::specs::consts;
namespace syntax = detail::specs::syntax;

////////////////////////////////////////////////////////////////////////////////
///                      Disassembling a constant value                      ///
////////////////////////////////////////////////////////////////////////////////

std::string Disassembler::Impl::GetUint32Value(const Segment& constants,
                                               size_t& pos) {
    const size_t start = pos;

    if (pos >= constants.size()) {
        throw DisassembleError::ConstantNoValue(start,
                                                consts::UINT32,
                                                pos,
                                                constants.size());
    }

    return std::to_string(constants[pos++]);
}

std::string Disassembler::Impl::GetUint64Value(const Segment& constants,
                                               size_t& pos) {
    const size_t start = pos;

    if (pos >= constants.size()) {
        throw DisassembleError::ConstantNoValue(start,
                                                consts::UINT64,
                                                pos,
                                                constants.size());
    }

    const arch::Word low = constants[pos++];

    if (pos >= constants.size()) {
        throw DisassembleError::ConstantNoValue(start,
                                                consts::UINT64,
                                                pos,
                                                constants.size());
    }

    const arch::Word high = constants[pos++];

    return std::to_string(utils::types::Join(low, high));
}

std::string Disassembler::Impl::GetDoubleValue(const Segment& constants,
                                               size_t& pos) {
    const size_t start = pos;

    if (pos >= constants.size()) {
        throw DisassembleError::ConstantNoValue(start,
                                                consts::DOUBLE,
                                                pos,
                                                constants.size());
    }

    const arch::Word low = constants[pos++];

    if (pos >= constants.size()) {
        throw DisassembleError::ConstantNoValue(start,
                                                consts::DOUBLE,
                                                pos,
                                                constants.size());
    }

    const arch::Word high = constants[pos++];

    auto value = std::bit_cast<consts::Double>(utils::types::Join(low, high));

    std::ostringstream ss;
    ss << std::setprecision(consts::kDoublePrecision) << value;

    return ss.str();
}

std::string Disassembler::Impl::GetCharValue(const Segment& constants,
                                             size_t& pos) {
    const size_t start = pos;

    if (pos >= constants.size()) {
        throw DisassembleError::ConstantNoValue(start,
                                                consts::DOUBLE,
                                                pos,
                                                constants.size());
    }

    const std::string character{static_cast<consts::Char>(constants[pos++])};

    return consts::kCharQuote + utils::strings::Escape(character) +
           consts::kCharQuote;
}

std::string Disassembler::Impl::GetStringValue(const Segment& constants,
                                               size_t& pos) {
    const size_t start = pos;

    if (pos >= constants.size()) {
        throw DisassembleError::ConstantNoValue(start,
                                                consts::STRING,
                                                pos,
                                                constants.size());
    }

    std::ostringstream ss;

    while (pos < constants.size() && constants[pos] != consts::kStringEnd) {
        ss << static_cast<consts::Char>(constants[pos++]);
    }

    if (pos == constants.size()) {
        throw DisassembleError::NoTrailingZeroInString(start);
    }

    // account for the '\0' at the end
    ++pos;

    return consts::kStringQuote + utils::strings::Escape(ss.str()) +
           consts::kStringQuote;
}

////////////////////////////////////////////////////////////////////////////////
///                    Disassembling the constants segment                   ///
////////////////////////////////////////////////////////////////////////////////

void Disassembler::Impl::DisassembleConstants(const Segment& constants,
                                              std::ostream& out,
                                              Labels& labels,
                                              size_t code_size) {
    size_t pos = 0;

    while (pos < constants.size()) {
        auto type = static_cast<consts::Type>(constants[pos++]);

        if (!consts::kTypeToName.contains(type)) {
            throw DisassembleError::UnknownConstantType(type);
        }

        auto curr_address = static_cast<arch::Address>(code_size + pos);
        const std::string current_label =
            labels.RecordConstantLabel(curr_address);

        std::string value;
        switch (type) {
            case consts::UINT32:
                value = GetUint32Value(constants, pos);
                break;

            case consts::UINT64:
                value = GetUint64Value(constants, pos);
                break;

            case consts::DOUBLE:
                value = GetDoubleValue(constants, pos);
                break;

            case consts::CHAR:
                value = GetCharValue(constants, pos);
                break;

            case consts::STRING:
                value = GetStringValue(constants, pos);
                break;

            default:
                throw InternalError::UnprocessedConstantType(type);
        }

        out << current_label << syntax::kLabelEnd << ' '
            << consts::kTypeToName.at(type) << ' ' << value << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////
///                          Disassembling a command                         ///
////////////////////////////////////////////////////////////////////////////////

std::string Disassembler::Impl::GetRegister(args::Register reg) {
    if (!arch::kRegisterNumToName.contains(reg)) {
        throw InternalError::RegisterNameNotFound(reg);
    }

    return arch::kRegisterNumToName.at(reg);
}

std::string Disassembler::Impl::GetCommandString(cmd::Bin command,
                                                 const Labels& labels) {
    std::ostringstream result;

    const cmd::Code code = cmd::GetCode(command);

    if (!cmd::kCodeToFormat.contains(code)) {
        throw DisassembleError::UnknownCommand(code);
    }

    if (!cmd::kCodeToName.contains(code)) {
        throw InternalError::CommandNameNotFound(code);
    }

    result << cmd::kCodeToName.at(code) << " ";

    switch (const cmd::Format format = cmd::kCodeToFormat.at(code)) {
        case cmd::RM: {
            const args::RMArgs args = cmd::parse::RM(command);

            result << GetRegister(args.reg) << " ";

            if (std::optional<std::string> label =
                    labels.TryGetLabel(args.addr)) {
                result << *label;
                break;
            }

            result << "0x" << std::hex << args.addr;

            break;
        }

        case cmd::RR: {
            const args::RRArgs args = cmd::parse::RR(command);

            result << GetRegister(args.recv) << " ";
            result << GetRegister(args.src) << " ";

            // two's complement logics is used per C++20 standard
            // see: https://urlis.net/kh43f3cf
            using SignedModifier = std::make_signed_t<args::Modifier>;
            result << static_cast<SignedModifier>(args.mod);

            break;
        }

        case cmd::RI: {
            const args::RIArgs args = cmd::parse::RI(command);

            result << GetRegister(args.reg) << " ";

            // two's complement logics is used per C++20 standard
            // see: https://urlis.net/kh43f3cf
            using SignedImmediate = std::make_signed_t<args::Immediate>;
            result << static_cast<SignedImmediate>(args.imm);

            break;
        }

        case cmd::J: {
            // do not confuse the reader with providing a label,
            // a non-zero value ot a hexadecimal '0x0' as an unused operand
            if (cmd::kJIgnoreAddress.contains(code)) {
                result << '0';
                break;
            }

            const args::JArgs args = cmd::parse::J(command);

            if (std::optional<std::string> label =
                    labels.TryGetLabel(args.addr)) {
                result << *label;
                break;
            }

            result << "0x" << std::hex << args.addr;

            break;
        }

        default: {
            throw InternalError::UnprocessedCommandFormat(format);
        }
    }

    return result.str();
}

////////////////////////////////////////////////////////////////////////////////
///                      Disassembling the code segment                      ///
////////////////////////////////////////////////////////////////////////////////

void Disassembler::Impl::DisassembleCode(const Segment& code,
                                         std::ostream& out,
                                         const Labels& labels) {
    for (size_t command_num = 0; command_num < code.size(); ++command_num) {
        auto curr_address = static_cast<arch::Address>(command_num);
        if (std::optional<std::string> label =
                labels.TryGetLabel(curr_address)) {
            out << std::endl << *label << syntax::kLabelEnd << std::endl;
        }

        out << "    " << GetCommandString(code[command_num], labels)
            << std::endl;
    }

    out << "end " << Labels::MainLabel() << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
///                          Disassembling to stream                         ///
////////////////////////////////////////////////////////////////////////////////

void Disassembler::Impl::DisassembleImpl(const std::string& src,
                                         std::ostream& out,
                                         std::ostream& log) {
    log << "[disassembler]: reading the executable file" << std::endl;

    const Exec::Data data = Exec::Read(src);

    log << "[disassembler]: successfully read the executable file" << std::endl;

    Labels labels;

    log << "[disassembler]: disassembling constants" << std::endl;

    DisassembleConstants(data.constants, out, labels, data.code.size());

    log << "[disassembler]: successfully disassembled constants" << std::endl;

    // newline between constants and code
    out << std::endl;

    log << "[disassembler]: preparing commands labels" << std::endl;

    labels.PrepareCommandLabels(data);

    log << "[disassembler]: successfully prepared commands labels" << std::endl;

    log << "[disassembler]: disassembling commands" << std::endl;

    DisassembleCode(data.code, out, labels);

    log << "[disassembler]: successfully disassembled commands" << std::endl;
}

void Disassembler::Impl::MustDisassemble(const std::string& src,
                                         std::ostream& out,
                                         std::ostream& log) {
    using std::string_literals::operator""s;

    try {
        DisassembleImpl(src, out, log);
    } catch (const errors::disassembler::Error& e) {
        log << "[disassembler]: error: " << e.what() << std::endl;
        throw e;
    } catch (const errors::Error& e) {
        log << "[disassembler]: error: " << e.what() << std::endl;
        throw errors::disassembler::Error(
            "error during disassembling process "
            "(not directly related to the disassembling itself): "s +
            e.what());
    } catch (const std::exception& e) {
        log << "[disassembler]: unexpected exception: " << e.what()
            << std::endl;
        throw errors::disassembler::Error(
            "unexpected exception in disassembler: "s + e.what());
    } catch (...) {
        log << "[disassembler]: unexpected exception" << std::endl;
        throw errors::disassembler::Error(
            "unexpected exception in disassembler "
            "(no additional info can be provided)");
    }
}

void Disassembler::Impl::Disassemble(const std::string& src,
                                     std::ostream& out,
                                     std::ostream& log) {
    try {
        MustDisassemble(src, out, log);
    } catch (const errors::Error& e) {
        std::cerr << e.what() << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////
///                           Disassembling to file                          ///
////////////////////////////////////////////////////////////////////////////////

void Disassembler::Impl::DisassembleImpl(const std::string& src,
                                         const std::string& dst,
                                         std::ostream& log) {
    std::string real_dst = dst;
    if (real_dst.empty()) {
        const std::filesystem::path src_path(src);

        std::filesystem::path dst_path = src_path.parent_path();
        dst_path /= src_path.stem();

        real_dst = dst_path.string() + "_disassembled.krm";
    }

    std::ofstream out(real_dst);
    if (out.fail()) {
        throw InternalError::FailedToOpen(real_dst);
    }

    return DisassembleImpl(src, out, log);
}

void Disassembler::Impl::MustDisassemble(const std::string& src,
                                         const std::string& dst,
                                         std::ostream& log) {
    using std::string_literals::operator""s;

    try {
        DisassembleImpl(src, dst, log);
    } catch (const errors::disassembler::Error& e) {
        log << "[disassembler]: error: " << e.what() << std::endl;
        throw e;
    } catch (const errors::Error& e) {
        log << "[disassembler]: error: " << e.what() << std::endl;
        throw errors::disassembler::Error(
            "error during disassembling process: "
            "(not directly related to the disassembling itself): "s +
            e.what());
    } catch (const std::exception& e) {
        log << "[disassembler]: unexpected exception: " << e.what()
            << std::endl;
        throw errors::disassembler::Error(
            "unexpected exception in disassembler: "s + e.what());
    } catch (...) {
        log << "[disassembler]: unexpected exception" << std::endl;
        throw errors::disassembler::Error(
            "unexpected exception in disassembler "
            "(no additional info can be provided)");
    }
}

void Disassembler::Impl::Disassemble(const std::string& src,
                                     const std::string& dst,
                                     std::ostream& log) {
    try {
        MustDisassemble(src, dst, log);
    } catch (const errors::Error& e) {
        std::cerr << e.what() << std::endl;
    }
}

}  // namespace karma
