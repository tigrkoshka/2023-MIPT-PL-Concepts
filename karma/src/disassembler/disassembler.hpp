#pragma once

#include <ostream>  // for ostream
#include <string>   // for string

namespace karma::disassembler {

void MustDisassemble(const std::string& exec_path, std::ostream& out);

void Disassemble(const std::string& exec_path, std::ostream& out);

void MustDisassemble(const std::string& exec_path, const std::string& dst = "");

void Disassemble(const std::string& exec_path, const std::string& dst = "");

}  // namespace karma::disassembler
