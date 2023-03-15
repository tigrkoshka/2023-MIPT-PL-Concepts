#include "compiler.hpp"

#include <string>  // for string

#include "compiler/impl.hpp"

namespace karma {

void Compiler::MustCompile(const std::string& src, const std::string& dst) {
    Impl::MustCompile(src, dst);
}

void Compiler::Compile(const std::string& src, const std::string& dst) {
    Impl::Compile(src, dst);
}

}  // namespace karma
