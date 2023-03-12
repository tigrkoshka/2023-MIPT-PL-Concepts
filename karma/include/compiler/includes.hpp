#pragma once

#include <memory>  // for unique_ptr
#include <string>  // for string
#include <vector>  // for vector

#include "file.hpp"

namespace karma::compiler::detail {

std::vector<std::unique_ptr<File>> GetFiles(const std::string& root_path);

}  // namespace karma::compiler::detail