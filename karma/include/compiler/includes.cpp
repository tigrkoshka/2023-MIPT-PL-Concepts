#include "includes.hpp"

#include <filesystem>  // for path
#include <memory>      // for unique_ptr
#include <string>      // for string
#include <utility>     // for move

#include "compiler/compiler.hpp"
#include "compiler/file.hpp"
#include "specs/syntax.hpp"

namespace karma {

namespace syntax = detail::specs::syntax;

std::vector<std::filesystem::path> Compiler::IncludesManager::GetIncludes(
    const std::unique_ptr<File>& file) {
    std::string token;
    std::vector<std::filesystem::path> includes;

    file->Open();
    while (file->NextLine()) {
        if (!file->GetToken(token)) {
            continue;
        }

        if (token != syntax::kIncludeDirective) {
            break;
        }

        if (!file->GetLine(token)) {
            throw CompileError::IncludeNoFilename(file->Where());
        }

        includes.emplace_back(token);
    }
    file->Close();

    return includes;
}

void Compiler::IncludesManager::ProcessFileIncludes(
    const std::unique_ptr<File>& file) {
    for (const auto& rel_include : GetIncludes(file)) {
        const std::filesystem::path include = std::filesystem::weakly_canonical(
            file->Path().parent_path() / rel_include);

        if (all_includes_.contains(include)) {
            continue;
        }

        all_includes_.insert(include);
        auto included_file = std::make_unique<File>(include, file.get());

        ProcessFileIncludes(included_file);

        files_.push_back(std::move(included_file));
    }
}

std::vector<std::unique_ptr<Compiler::File>>
Compiler::IncludesManager::GetFiles(const std::string& root) && {
    const std::filesystem::path abs_root =
        std::filesystem::weakly_canonical(root);

    all_includes_.insert(abs_root);
    auto root_file = std::make_unique<File>(abs_root);

    ProcessFileIncludes(root_file);

    files_.push_back(std::move(root_file));

    return std::move(files_);
}

}  // namespace karma
