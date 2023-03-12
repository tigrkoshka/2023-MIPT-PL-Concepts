#pragma once

#include <string>  // for string

namespace karma::compiler {

void MustCompile(const std::string& src, const std::string& dst = "");
void Compile(const std::string& src, const std::string& dst = "");

}  // namespace karma::compiler