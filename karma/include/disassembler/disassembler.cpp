#include "disassembler.hpp"

#include <ostream>  // for ostream
#include <string>   // for string

#include "disassembler/impl.hpp"

namespace karma {

void Disassembler::MustDisassemble(const std::string& exec_path,
                                   std::ostream& out) {
    Impl::MustDisassemble(exec_path, out);
}

void Disassembler::Disassemble(const std::string& exec_path,
                               std::ostream& out) {
    Impl::Disassemble(exec_path, out);
}

void Disassembler::MustDisassemble(const std::string& exec_path,
                                   const std::string& dst) {
    Impl::MustDisassemble(exec_path, dst);
}

void Disassembler::Disassemble(const std::string& exec_path,
                               const std::string& dst) {
    Impl::Disassemble(exec_path, dst);
}

}  // namespace karma
