#include "impl.hpp"

#include <bit>          // for bit_cast
#include <exception>    // for exception
#include <filesystem>   // for path
#include <fstream>      // for ofstream
#include <iomanip>      // for setprecision
#include <iostream>     // for cout
#include <sstream>      // for ostringstream
#include <string>       // for string
#include <type_traits>  // for make_signed_t

#include "exec/exec.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/constants.hpp"
#include "utils/error.hpp"
#include "utils/strings.hpp"
#include "utils/types.hpp"

namespace karma {

namespace utils  = detail::utils;
namespace arch   = detail::specs::arch;
namespace cmd    = detail::specs::cmd;
namespace args   = cmd::args;
namespace consts = detail::specs::consts;

////////////////////////////////////////////////////////////////////////////////
///                      Disassembling a constant value                      ///
////////////////////////////////////////////////////////////////////////////////

std::string Disassembler::Impl::GetUint32Value(const Segment& constants,
                                               size_t& pos) {
    size_t start = pos;

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
    size_t start = pos;

    if (pos >= constants.size()) {
        throw DisassembleError::ConstantNoValue(start,
                                                consts::UINT64,
                                                pos,
                                                constants.size());
    }

    arch::Word low = constants[pos++];

    if (pos >= constants.size()) {
        throw DisassembleError::ConstantNoValue(start,
                                                consts::UINT64,
                                                pos,
                                                constants.size());
    }

    arch::Word high = constants[pos++];

    consts::UInt64 value = utils::types::Join(low, high);

    return std::to_string(value);
}

std::string Disassembler::Impl::GetDoubleValue(const Segment& constants,
                                               size_t& pos) {
    size_t start = pos;

    if (pos >= constants.size()) {
        throw DisassembleError::ConstantNoValue(start,
                                                consts::DOUBLE,
                                                pos,
                                                constants.size());
    }

    arch::Word low = constants[pos++];

    if (pos >= constants.size()) {
        throw DisassembleError::ConstantNoValue(start,
                                                consts::DOUBLE,
                                                pos,
                                                constants.size());
    }

    arch::Word high = constants[pos++];

    auto value = std::bit_cast<consts::Double>(utils::types::Join(low, high));

    std::ostringstream ss;
    ss << std::setprecision(consts::kDoublePrecision) << value;

    return ss.str();
}

std::string Disassembler::Impl::GetCharValue(const Segment& constants,
                                             size_t& pos) {
    size_t start = pos;

    if (pos >= constants.size()) {
        throw DisassembleError::ConstantNoValue(start,
                                                consts::DOUBLE,
                                                pos,
                                                constants.size());
    }

    std::string character{static_cast<consts::Char>(constants[pos++])};

    return consts::kCharQuote + utils::strings::Escape(character) +
           consts::kCharQuote;
}

std::string Disassembler::Impl::GetStringValue(const Segment& constants,
                                               size_t& pos) {
    size_t start = pos;

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
                                              std::ostream& out) {
    size_t pos = 0;

    while (pos < constants.size()) {
        auto type = static_cast<consts::Type>(constants[pos++]);

        if (!consts::kTypeToName.contains(type)) {
            throw DisassembleError::UnknownConstantType(type);
        }

        std::string value{};

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

        out << consts::kTypeToName.at(type) << " " << value << std::endl;
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

std::string Disassembler::Impl::GetCommandString(cmd::Bin command) {
    std::ostringstream result;

    cmd::Code code = cmd::GetCode(command);

    if (!cmd::kCodeToFormat.contains(code)) {
        throw DisassembleError::UnknownCommand(code);
    }

    if (!cmd::kCodeToName.contains(code)) {
        throw InternalError::CommandNameNotFound(code);
    }

    result << cmd::kCodeToName.at(code) << " ";

    switch (cmd::Format format = cmd::kCodeToFormat.at(code)) {
        case cmd::RM: {
            args::RMArgs args = cmd::parse::RM(command);

            result << GetRegister(args.reg) << " ";
            result << std::hex << args.addr;

            break;
        }

        case cmd::RR: {
            args::RRArgs args = cmd::parse::RR(command);

            result << GetRegister(args.recv) << " ";
            result << GetRegister(args.src) << " ";

            // two's complement logics is used per C++20 standard
            // see: https://urlis.net/kh43f3cf
            using SignedModifier = std::make_signed_t<args::Modifier>;
            result << static_cast<SignedModifier>(args.mod);

            break;
        }

        case cmd::RI: {
            args::RIArgs args = cmd::parse::RI(command);

            result << GetRegister(args.reg) << " ";

            // two's complement logics is used per C++20 standard
            // see: https://urlis.net/kh43f3cf
            using SignedImmediate = std::make_signed_t<args::Immediate>;
            result << static_cast<SignedImmediate>(args.imm);

            break;
        }

        case cmd::J: {
            args::JArgs args = cmd::parse::J(command);

            result << std::hex << args.addr;

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
                                         size_t entrypoint,
                                         std::ostream& out) {
    for (size_t i = 0; i < entrypoint; ++i) {
        out << GetCommandString(code[i]) << std::endl;
    }

    if (entrypoint != 0) {
        out << std::endl;
    }

    out << "main:" << std::endl;

    for (size_t i = entrypoint; i < code.size(); ++i) {
        out << "    " << GetCommandString(code[i]) << std::endl;
    }

    out << "end main" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
///                          Disassembling to stream                         ///
////////////////////////////////////////////////////////////////////////////////

void Disassembler::Impl::DisassembleImpl(const std::string& exec_path,
                                         std::ostream& out) {
    Exec::Data data = Exec::Read(exec_path);

    DisassembleConstants(data.constants, out);

    out << std::endl;

    DisassembleCode(data.code, data.entrypoint, out);
}

void Disassembler::Impl::MustDisassemble(const std::string& exec_path,
                                         std::ostream& out) {
    DisassembleImpl(exec_path, out);
}

void Disassembler::Impl::Disassemble(const std::string& exec_path,
                                     std::ostream& out) {
    try {
        DisassembleImpl(exec_path, out);
    } catch (const errors::disassembler::Error& e) {
        std::cout << e.what() << std::endl;
    } catch (const errors::Error& e) {
        std::cout << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "disassembler: unexpected exception: " << e.what()
                  << std::endl;
    } catch (...) {
        std::cout << "disassembler: unexpected exception" << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////
///                           Disassembling to file                          ///
////////////////////////////////////////////////////////////////////////////////

void Disassembler::Impl::DisassembleImpl(const std::string& exec_path,
                                         const std::string& dst) {
    std::string real_dst = dst;
    if (real_dst.empty()) {
        std::filesystem::path src_path(exec_path);

        std::filesystem::path dst_path = src_path.parent_path();
        dst_path /= src_path.stem();

        real_dst = dst_path.string() + "_disassembled.krm";
    }

    std::ofstream out(real_dst);
    if (out.fail()) {
        throw InternalError::FailedToOpen(real_dst);
    }

    return DisassembleImpl(exec_path, out);
}

void Disassembler::Impl::MustDisassemble(const std::string& exec_path,
                                         const std::string& dst) {
    using std::string_literals::operator""s;

    try {
        DisassembleImpl(exec_path, dst);
    } catch (const errors::Error& e) {
        throw e;
    } catch (const std::exception& e) {
        throw errors::disassembler::Error(
            "unexpected disassembler exception: "s + e.what());
    } catch (...) {
        throw errors::disassembler::Error("unexpected disassembler exception");
    }
}

void Disassembler::Impl::Disassemble(const std::string& exec_path,
                                     const std::string& dst) {
    try {
        MustDisassemble(exec_path, dst);
    } catch (const errors::Error& e) {
        std::cout << e.what() << std::endl;
    }
}

}  // namespace karma