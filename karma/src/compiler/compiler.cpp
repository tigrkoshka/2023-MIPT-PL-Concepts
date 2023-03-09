#include "compiler.hpp"

#include <istream>  // for istream
#include <string>   // for string

#include "impl.hpp"

namespace karma::compiler {

void MustCompile(std::istream& code, const std::string& exec_path) {
    detail::Impl().MustCompile(code, exec_path);
}

void Compile(std::istream& code, const std::string& exec_path) {
    detail::Impl().Compile(code, exec_path);
}

void MustCompile(const std::string& src, const std::string& dst) {
    detail::Impl().MustCompile(src, dst);
}

void Compile(const std::string& src, const std::string& dst) {
    detail::Impl().Compile(src, dst);
}

}  // namespace karma::compiler
