#pragma once

#include <istream>  // for istream
#include <string>   // for string

namespace karma::compiler {

void MustCompile(std::istream& code, const std::string& exec_path);
void Compile(std::istream& code, const std::string& exec_path);

void MustCompile(const std::string& src, const std::string& dst = "");
void Compile(const std::string& src, const std::string& dst = "");

}  // namespace karma::compiler