#pragma once

#include <ostream>  // for ostream
#include <string>   // for string

#include "specs/commands.hpp"

namespace karma {

namespace detail::disassembler {

class Impl {
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
                                const std::string& dst);

    static void Disassemble(const std::string& exec_path,
                            const std::string& dst);
};

}  // namespace detail::disassembler

void MustDisassemble(const std::string& exec_path, std::ostream& out);

void Disassemble(const std::string& exec_path, std::ostream& out);

void MustDisassemble(const std::string& exec_path, const std::string& dst = "");

void Disassemble(const std::string& exec_path, const std::string& dst = "");

}  // namespace karma
