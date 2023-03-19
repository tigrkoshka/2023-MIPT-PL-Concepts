#pragma once

#include <cstddef>  // for size_t
#include <ostream>  // for ostream
#include <string>   // for string
#include <vector>   // for vector

#include "disassembler/disassembler.hpp"
#include "disassembler/errors.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "utils/traits.hpp"

namespace karma {

class Disassembler::Impl : detail::utils::traits::Static {
   private:
    using InternalError    = errors::disassembler::InternalError::Builder;
    using DisassembleError = errors::disassembler::DisassembleError::Builder;

    using Segment = std::vector<detail::specs::arch::Word>;

   private:
    static std::string GetUint32Value(const Segment& constants, size_t& pos);
    static std::string GetUint64Value(const Segment& constants, size_t& pos);
    static std::string GetDoubleValue(const Segment& constants, size_t& pos);
    static std::string GetCharValue(const Segment& constants, size_t& pos);
    static std::string GetStringValue(const Segment& constants, size_t& pos);

    static void DisassembleConstants(const Segment& constants,
                                     std::ostream& out,
                                     Labels& labels,
                                     size_t code_size);

    static std::string GetRegister(detail::specs::cmd::args::Register);

    static std::string GetCommandString(detail::specs::cmd::Bin, const Labels&);

    static void DisassembleCode(const Segment& code,
                                std::ostream& out,
                                const Labels& labels);

    static void DisassembleImpl(const std::string& exec_path,
                                std::ostream& out);

    static void DisassembleImpl(const std::string& exec_path,
                                const std::string& dst);

   public:
    static void MustDisassemble(const std::string& exec_path,
                                std::ostream& out);

    static void Disassemble(const std::string& exec_path, std::ostream& out);

    static void MustDisassemble(const std::string& exec_path,
                                const std::string& dst);

    static void Disassemble(const std::string& exec_path,
                            const std::string& dst);
};

}  // namespace karma
