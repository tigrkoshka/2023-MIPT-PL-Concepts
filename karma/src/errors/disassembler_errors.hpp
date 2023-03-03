#pragma once

#include <string>   // for string, to_string

#include "../specs/commands.hpp"
#include "error.hpp"

namespace karma::errors::disassembler {

struct Error : errors::Error {
   protected:
    explicit Error(const std::string& message)
        : errors::Error(message) {}
};

struct InternalError : Error {
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

struct DisassembleError : Error {
   private:
    explicit DisassembleError(const std::string& message)
        : Error("disassembling error: " + message) {}

   public:
    static DisassembleError UnknownCommand(detail::specs::cmd::Code);
};

}  // namespace karma::errors::disassembler