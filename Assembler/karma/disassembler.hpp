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

   public:
    Error(const Error&)            = default;
    Error& operator=(const Error&) = default;
    Error(Error&&)                 = default;
    Error& operator=(Error&&)      = default;
    ~Error() override              = default;
};

struct Disassembler::InternalError : Error {
   protected:
    explicit InternalError(const std::string& message)
        : Error("internal disassembler error: " + message) {}

   public:
    InternalError(const InternalError&)            = default;
    InternalError& operator=(const InternalError&) = default;
    InternalError(InternalError&&)                 = default;
    InternalError& operator=(InternalError&&)      = default;
    ~InternalError() override                      = default;

    static InternalError FailedToOpen(const std::string& path);

    static InternalError CommandNameNotFound(detail::specs::cmd::Code);

    static InternalError RegisterNameNotFound(
        detail::specs::cmd::args::Register);

    static InternalError UnknownCommandFormat(detail::specs::cmd::Format);
};

struct Disassembler::DisassembleError : Error {
   protected:
    explicit DisassembleError(const std::string& message)
        : Error("disassembling error: " + message) {}

   public:
    DisassembleError(const DisassembleError&)            = default;
    DisassembleError& operator=(const DisassembleError&) = default;
    DisassembleError(DisassembleError&&)                 = default;
    DisassembleError& operator=(DisassembleError&&)      = default;
    ~DisassembleError() override                         = default;

    static DisassembleError UnknownCommand(detail::specs::cmd::Code);
};

}  // namespace karma
