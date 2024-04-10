#include "disassembler.hpp"

#include <ostream>  // for ostream
#include <string>   // for string

#include "disassembler/impl.hpp"
#include "utils/logger.hpp"

namespace karma {

void Disassembler::MustDisassemble(const std::string& src,
                                   const std::string& dst,
                                   Logger log) {
    Impl::MustDisassemble(src, dst, log.log);
}

void Disassembler::Disassemble(const std::string& src,
                               const std::string& dst,
                               Logger log) {
    Impl::Disassemble(src, dst, log.log);
}

void Disassembler::MustDisassemble(const std::string& src,
                                   std::ostream& out,
                                   Logger log) {
    Impl::MustDisassemble(src, out, log.log);
}

void Disassembler::Disassemble(const std::string& src,
                               std::ostream& out,
                               Logger log) {
    Impl::Disassemble(src, out, log.log);
}

void Disassembler::MustDisassemble(const std::string& src, Logger log) {
    MustDisassemble(src, "", log);
}

void Disassembler::Disassemble(const std::string& src, Logger log) {
    Disassemble(src, "", log);
}

}  // namespace karma
