#include "disassembler.hpp"

#include <ostream>  // for ostream
#include <string>   // for string

#include "disassembler/impl.hpp"
#include "utils/logger.hpp"

namespace karma {

std::ostream& Disassembler::no_op_logger = detail::utils::logger::no_op;

void Disassembler::MustDisassemble(const std::string& exec_path,
                                   std::ostream& out,
                                   std::ostream& log) {
    Impl::MustDisassemble(exec_path, out, log);
}

void Disassembler::Disassemble(const std::string& exec_path,
                               std::ostream& out,
                               std::ostream& log) {
    Impl::Disassemble(exec_path, out, log);
}

void Disassembler::MustDisassemble(const std::string& exec_path,
                                   const std::string& dst,
                                   std::ostream& log) {
    Impl::MustDisassemble(exec_path, dst, log);
}

void Disassembler::Disassemble(const std::string& exec_path,
                               const std::string& dst,
                               std::ostream& log) {
    Impl::Disassemble(exec_path, dst, log);
}

}  // namespace karma
