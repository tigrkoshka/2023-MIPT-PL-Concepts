#include "disassembler.hpp"

#include <exception>    // for exception
#include <filesystem>   // for path
#include <fstream>      // for ofstream
#include <iostream>     // for ostream, cout
#include <sstream>      // for ostringstream
#include <string>       // for string
#include <type_traits>  // for make_signed_t

#include "errors/disassembler_errors.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/exec.hpp"

namespace karma {

using namespace errors::disassembler;  // NOLINT(google-build-using-namespace)

namespace arch = detail::specs::arch;

namespace cmd  = detail::specs::cmd;
namespace args = cmd::args;

namespace exec = detail::specs::exec;

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
///                          Disassembling to stream                         ///
////////////////////////////////////////////////////////////////////////////////

void Disassembler::DisassembleImpl(const std::string& exec_path,
                                   std::ostream& out) {
    out << "main:\n";

    exec::Data data = exec::Read(exec_path);
    for (cmd::Bin command : data.code) {
        out << "  " << GetCommandStringFromBin(command) << "\n";
    }

    out << "end main\n";
}

void Disassembler::MustDisassemble(const std::string& exec_path,
                                   std::ostream& out) {
    DisassembleImpl(exec_path, out);
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

////////////////////////////////////////////////////////////////////////////////
///                           Disassembling to file                          ///
////////////////////////////////////////////////////////////////////////////////

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void Disassembler::DisassembleImpl(const std::string& exec_path,
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

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void Disassembler::MustDisassemble(const std::string& exec_path,
                                   const std::string& dst) {
    return DisassembleImpl(exec_path, dst);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void Disassembler::Disassemble(const std::string& exec_path,
                               const std::string& dst) {
    try {
        DisassembleImpl(exec_path, dst);
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