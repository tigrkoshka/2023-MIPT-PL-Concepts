#include "includes.hpp"

#include <filesystem>  // for path
#include <memory>      // for unique_ptr
#include <string>      // for string
#include <utility>     // for move

#include "compiler/errors.hpp"
#include "compiler/file.hpp"
#include "specs/syntax.hpp"

namespace karma::compiler::detail {

using karma::errors::compiler::InternalError;
using karma::errors::compiler::CompileError;

namespace syntax = karma::detail::specs::syntax;

IncludesManager::Includes IncludesManager::GetCurrIncludes() {
    std::string token;

    const std::unique_ptr<File>& curr_file = files_.back();

    Includes includes;

    curr_file->Open();
    while (curr_file->NextLine()) {
        if (!curr_file->GetToken(token)) {
            continue;
        }

        if (token != syntax::kIncludeDirective) {
            break;
        }

        if (!curr_file->GetLine(token)) {
            throw CompileError::IncludeNoFilename(curr_file->Where());
        }

        includes.emplace_back(token);
    }
    curr_file->Close();

    return includes;
}

void IncludesManager::ProcessCurrFileIncludes() {
    const std::unique_ptr<File>& curr_file = files_.back();

    for (const auto& rel_include : GetCurrIncludes()) {
        std::string include =
            files_.size() == 1 ? abs_root_dir_ / rel_include
                               : curr_file->Path().parent_path() / rel_include;

        if (all_includes_.contains(include)) {
            continue;
        }

        all_includes_.insert(include);
        files_.push_back(std::make_unique<File>(include, curr_file.get()));

        ProcessCurrFileIncludes();
    }
}

IncludesManager::Files IncludesManager::GetFiles(const std::string& root) && {
    std::filesystem::path abs_root = std::filesystem::absolute(root);

    abs_root_dir_ = abs_root.parent_path();
    all_includes_.insert(abs_root);
    files_.push_back(std::make_unique<File>(abs_root));

    ProcessCurrFileIncludes();

    return std::move(files_);
}

IncludesManager::Files GetFiles(const std::string& root) {
    return IncludesManager().GetFiles(root);
}

}  // namespace karma::compiler::detail
