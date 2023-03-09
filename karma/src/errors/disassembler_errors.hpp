#pragma once

#include <string>  // for string, to_string

#include "../specs/commands.hpp"
#include "../specs/constants.hpp"
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

    static InternalError UnprocessedConstantType(detail::specs::consts::Type);

    static InternalError CommandNameNotFound(detail::specs::cmd::Code);

    static InternalError RegisterNameNotFound(
        detail::specs::cmd::args::Register);

    static InternalError UnprocessedCommandFormat(detail::specs::cmd::Format);
};

struct DisassembleError : Error {
   private:
    explicit DisassembleError(const std::string& message)
        : Error("disassembling error: " + message) {}

   public:
    static DisassembleError UnknownConstantType(detail::specs::consts::Type);

    static DisassembleError ConstantNoValue(
        size_t start,
        detail::specs::consts::Type expected,
        size_t index,
        size_t constants_size);

    static DisassembleError NoTrailingZeroInString(size_t start);

    static DisassembleError UnknownCommand(detail::specs::cmd::Code);
};

}  // namespace karma::errors::disassembler