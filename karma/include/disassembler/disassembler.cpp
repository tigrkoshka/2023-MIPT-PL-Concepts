#include "disassembler.hpp"

#include "impl.hpp"

namespace karma::disassembler {

void MustDisassemble(const std::string& exec_path, std::ostream& out) {
    detail::Impl::MustDisassemble(exec_path, out);
}

void Disassemble(const std::string& exec_path, std::ostream& out) {
    detail::Impl::Disassemble(exec_path, out);
}

void MustDisassemble(const std::string& exec_path, const std::string& dst) {
    detail::Impl::MustDisassemble(exec_path, dst);
}

void Disassemble(const std::string& exec_path, const std::string& dst) {
    detail::Impl::Disassemble(exec_path, dst);
}

}  // namespace karma::disassembler