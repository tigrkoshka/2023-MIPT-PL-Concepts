#include "compiler.hpp"

#include <ostream>  // for ostream
#include <string>   // for string

#include "compiler/impl.hpp"
#include "utils/logger.hpp"

namespace karma {

std::ostream& Compiler::NoOpLogger = detail::utils::logger::no_op;

void Compiler::MustCompile(const std::string& src,
                           const std::string& dst,
                           std::ostream& log) {
    Impl::MustCompile(src, dst, log);
}

void Compiler::Compile(const std::string& src,
                       const std::string& dst,
                       std::ostream& log) {
    Impl::Compile(src, dst, log);
}

}  // namespace karma
