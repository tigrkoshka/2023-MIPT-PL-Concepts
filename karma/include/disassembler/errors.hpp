#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string, to_string

#include "disassembler/disassembler.hpp"
#include "specs/commands.hpp"
#include "specs/constants.hpp"
#include "utils/traits.hpp"

namespace karma::errors::disassembler {

struct InternalError::Builder : detail::utils::traits::Static {
    static InternalError FailedToOpen(const std::string& path);

    static InternalError UnprocessedConstantType(detail::specs::consts::Type);

    static InternalError CommandNameNotFound(detail::specs::cmd::Code);

    static InternalError RegisterNameNotFound(
        detail::specs::cmd::args::Register);

    static InternalError UnprocessedCommandFormat(detail::specs::cmd::Format);
};

struct DisassembleError::Builder : detail::utils::traits::Static {
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