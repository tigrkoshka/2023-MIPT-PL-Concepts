#pragma once

#include <filesystem>     // for path
#include <memory>         // for unique_ptr
#include <string>         // for string
#include <unordered_set>  // for unordered_set
#include <vector>         // for vector

#include "compiler/compiler.hpp"
#include "compiler/file.hpp"
#include "utils/traits.hpp"

namespace karma {

class Compiler::IncludesManager
    : karma::detail::utils::traits::NonCopyableMovable {
   private:
    using InternalError = errors::compiler::InternalError::Builder;
    using CompileError = errors::compiler::CompileError::Builder;

    using Files    = std::vector<std::unique_ptr<File>>;
    using Includes = std::vector<std::filesystem::path>;

   private:
    Includes GetCurrIncludes();

    void ProcessCurrFileIncludes();

   public:
    Files GetFiles(const std::string& root) &&;

   private:
    std::filesystem::path abs_root_dir_;
    std::unordered_set<std::string> all_includes_;
    Files files_;
};

}  // namespace karma::compiler::detail