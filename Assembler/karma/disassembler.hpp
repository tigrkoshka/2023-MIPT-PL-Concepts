#pragma once

#include <iostream>   // for ostream, cout
#include <stdexcept>  // for runtime_error
#include <string>     // for string

#include "specs/commands.hpp"

namespace karma {

class Disassembler {
   public:
    struct Error;
    struct InternalError;
    struct DisassembleError;

   private:
    static std::string GetRegister(detail::specs::cmd::args::Register);

    static std::string GetCommandStringFromBin(detail::specs::cmd::Bin);

    static void DisassembleImpl(const std::string& exec_path,
                                std::ostream& out);

    static void DisassembleImpl(const std::string& exec_path,
                                const std::string& dst);

   public:
    static void MustDisassemble(const std::string& exec_path,
                                std::ostream& out);

    static void Disassemble(const std::string& exec_path, std::ostream& out);

    static void MustDisassemble(const std::string& exec_path,
                                const std::string& dst = "");

    static void Disassemble(const std::string& exec_path,
                            const std::string& dst = "");
};

struct Disassembler::Error : std::runtime_error {
   protected:
    explicit Error(const std::string& message)
        : std::runtime_error(message) {}
};

struct Disassembler::InternalError : Error {
   private:
    explicit InternalError(const std::string& message)
        : Error("internal disassembler error: " + message) {}

   public:
    static InternalError FailedToOpen(const std::string& path);

    static InternalError CommandNameNotFound(detail::specs::cmd::Code);

    static InternalError RegisterNameNotFound(
        detail::specs::cmd::args::Register);

    static InternalError UnknownCommandFormat(detail::specs::cmd::Format);
};

struct Disassembler::DisassembleError : Error {
   private:
    explicit DisassembleError(const std::string& message)
        : Error("disassembling error: " + message) {}

   public:
    static DisassembleError UnknownCommand(detail::specs::cmd::Code);
};

}  // namespace karma
