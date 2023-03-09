#pragma once

#include <cstddef>  // for size_t
#include <ostream>  // for ostream
#include <string>   // for string
#include <vector>   // for vector

#include "specs/architecture.hpp"
#include "specs/commands.hpp"

namespace karma::disassembler::detail {

class Impl {
   private:
    using Segment = std::vector<karma::detail::specs::arch::Word>;

   private:
    static std::string GetUint32Value(const Segment& constants, size_t& pos);
    static std::string GetUint64Value(const Segment& constants, size_t& pos);
    static std::string GetDoubleValue(const Segment& constants, size_t& pos);
    static std::string GetCharValue(const Segment& constants, size_t& pos);
    static std::string GetStringValue(const Segment& constants, size_t& pos);

    static void DisassembleConstants(const Segment& constants,
                                     std::ostream& out);

    static std::string GetRegister(karma::detail::specs::cmd::args::Register);

    static std::string GetCommandString(karma::detail::specs::cmd::Bin);

    static void DisassembleCode(const Segment& code,
                                size_t entrypoint,
                                std::ostream& out);

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

}  // namespace karma::disassembler::detail