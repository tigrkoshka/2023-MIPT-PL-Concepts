#include "compiler.hpp"

#include <string>  // for string

#include "compiler/impl.hpp"

namespace karma::compiler {

void MustCompile(const std::string& src, const std::string& dst) {
    detail::Impl().MustCompile(src, dst);
}

void Compile(const std::string& src, const std::string& dst) {
    detail::Impl().Compile(src, dst);
}

}  // namespace karma::compiler
