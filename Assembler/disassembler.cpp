#include "disassembler.hpp"

#include <exception>    // for exception
#include <ostream>      // for ostream
#include <sstream>      // for ostringstream
#include <string>       // for string
#include <type_traits>  // for make_signed_t

#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/exec.hpp"

namespace karma {

namespace arch = detail::specs::arch;

namespace cmd  = detail::specs::cmd;
namespace args = cmd::args;

namespace exec = detail::specs::exec;

////////////////////////////////////////////////////////////////////////////////
///                              Internal errors                             ///
////////////////////////////////////////////////////////////////////////////////

using InternalError = Disassembler::InternalError;

InternalError InternalError::CommandNameNotFound(cmd::Code code) {
    std::ostringstream ss;
    ss << "name not found for command with code " << code;
    return InternalError{ss.str()};
}

InternalError InternalError::RegisterNameNotFound(args::Register reg) {
    std::ostringstream ss;
    ss << "name not found for register # " << reg;
    return InternalError{ss.str()};
}

InternalError InternalError::UnknownCommandFormat(cmd::Format format) {
    std::ostringstream ss;
    ss << "unknown command format " << format;
    return InternalError{ss.str()};
}

////////////////////////////////////////////////////////////////////////////////
///                           Disassembling errors                           ///
////////////////////////////////////////////////////////////////////////////////

using DisassembleError = Disassembler::DisassembleError;

DisassembleError DisassembleError::UnknownCommand(cmd::Code code) {
    std::ostringstream ss;
    ss << "unknown command code " << code;
    return DisassembleError{ss.str()};
}

////////////////////////////////////////////////////////////////////////////////
///                          Disassembling a command                         ///
////////////////////////////////////////////////////////////////////////////////

std::string Disassembler::GetRegister(args::Register reg) {
    if (!arch::kRegisterNumToName.contains(reg)) {
        throw InternalError::RegisterNameNotFound(reg);
    }

    return arch::kRegisterNumToName.at(reg);
}

std::string Disassembler::GetCommandStringFromBin(cmd::Bin command) {
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
            auto [reg, addr] = cmd::parse::RM(command);

            result << GetRegister(reg) << " ";
            result << std::hex << addr;

            break;
        }

        case cmd::RR: {
            auto [recv, src, mod] = cmd::parse::RR(command);

            result << GetRegister(recv) << " ";
            result << GetRegister(src) << " ";

            // two's complement logics is used per C++20 standard
            // see: https://urlis.net/kh43f3cf
            result << static_cast<std::make_signed_t<args::Modifier>>(mod);

            break;
        }

        case cmd::RI: {
            auto [reg, imm] = cmd::parse::RI(command);

            result << GetRegister(reg) << " ";

            // two's complement logics is used per C++20 standard
            // see: https://urlis.net/kh43f3cf
            result << static_cast<std::make_signed_t<args::Immediate>>(imm);

            break;
        }

        case cmd::J: {
            auto [addr] = cmd::parse::J(command);

            result << std::hex << addr;

            break;
        }

        default: {
            throw InternalError::UnknownCommandFormat(format);
        }
    }

    return result.str();
}

////////////////////////////////////////////////////////////////////////////////
///                          Disassembling exec file                         ///
////////////////////////////////////////////////////////////////////////////////

void Disassembler::DisassembleImpl(const std::string& exec_path,
                                   std::ostream& out) {
    exec::Data data = exec::Read(exec_path);
    for (cmd::Bin command : data.code) {
        out << GetCommandStringFromBin(command) << "\n";
    }
}

void Disassembler::Disassemble(const std::string& exec_path,
                               std::ostream& out) {
    try {
        DisassembleImpl(exec_path, out);
    } catch (const Error& e) {
        std::cout << e.what() << std::endl;
    } catch (const exec::Error& e) {
        std::cout << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "disassembler: unexpected exception: " << e.what()
                  << std::endl;
    } catch (...) {
        std::cout << "disassembler: unexpected exception" << std::endl;
    }
}

}  // namespace karma