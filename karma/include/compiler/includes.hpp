#pragma once

#include <filesystem>     // for path
#include <memory>         // for unique_ptr
#include <string>         // for string
#include <unordered_set>  // for unordered_set
#include <vector>         // for vector

#include "compiler/compiler.hpp"
#include "compiler/errors.hpp"
#include "compiler/file.hpp"
#include "utils/traits.hpp"

namespace karma {

class Compiler::IncludesManager : detail::utils::traits::NonCopyableMovable {
   private:
    using InternalError = errors::compiler::InternalError::Builder;
    using CompileError  = errors::compiler::CompileError::Builder;

   private:
    static std::vector<std::filesystem::path> GetIncludes(
        const std::unique_ptr<File>&);

    void ProcessFileIncludes(const std::unique_ptr<File>&);

   public:
    std::vector<std::unique_ptr<File>> GetFiles(const std::string& root) &&;

   private:
    std::unordered_set<std::string> all_includes_{};
    std::vector<std::unique_ptr<File>> files_{};
};

}  // namespace karma