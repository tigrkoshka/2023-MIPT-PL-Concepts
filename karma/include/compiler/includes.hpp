#pragma once

#include <filesystem>     // for path
#include <memory>         // for unique_ptr
#include <string>         // for string
#include <unordered_set>  // for unordered_set
#include <vector>         // for vector

#include "compiler/file.hpp"
#include "utils/traits.hpp"

namespace karma::compiler::detail {

class IncludesManager : karma::detail::utils::traits::NonCopyableMovable {
   private:
    using Files    = std::vector<std::unique_ptr<File>>;
    using Includes = std::vector<std::filesystem::path>;

   private:
    friend Files GetFiles(const std::string& root);

   private:
    Includes GetCurrIncludes();

    void ProcessCurrFileIncludes();

    Files GetFiles(const std::string& root) &&;

   private:
    std::filesystem::path abs_root_dir_;
    std::unordered_set<std::string> all_includes_;
    Files files_;
};

IncludesManager::Files GetFiles(const std::string& root);

}  // namespace karma::compiler::detail