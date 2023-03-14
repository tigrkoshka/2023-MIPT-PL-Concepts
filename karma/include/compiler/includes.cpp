#include "includes.hpp"

#include <filesystem>     // for path
#include <memory>         // for unique_ptr
#include <string>         // for string
#include <unordered_set>  // for unordered_set
#include <utility>        // for move
#include <vector>         // for vector

#include "errors.hpp"
#include "file.hpp"
#include "specs/syntax.hpp"
#include "utils/traits.hpp"

namespace karma::compiler::detail {

namespace fs = std::filesystem;

using Files    = std::vector<std::unique_ptr<File>>;
using Includes = std::vector<fs::path>;

using karma::errors::compiler::InternalError;
using karma::errors::compiler::CompileError;

namespace syntax = karma::detail::specs::syntax;

class IncludesManager : karma::detail::utils::traits::NonCopyableMovable {
   private:
    Includes GetCurrIncludes() {
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

    void ProcessCurrFileIncludes() {
        const std::unique_ptr<File>& curr_file = files_.back();

        for (const auto& rel_include : GetCurrIncludes()) {
            std::string include =
                files_.size() == 1
                    ? abs_root_dir_ / rel_include
                    : curr_file->Path().parent_path() / rel_include;

            if (all_includes_.contains(include)) {
                continue;
            }

            all_includes_.insert(include);
            files_.push_back(std::make_unique<File>(include, curr_file.get()));

            ProcessCurrFileIncludes();
        }
    }

   public:
    Files GetFiles(const std::string& root_path) && {
        fs::path abs_root_path = fs::absolute(root_path);

        abs_root_dir_ = abs_root_path.parent_path();
        all_includes_.insert(abs_root_path);
        files_.push_back(std::make_unique<File>(abs_root_path));

        ProcessCurrFileIncludes();

        return std::move(files_);
    }

   private:
    fs::path abs_root_dir_;
    std::unordered_set<std::string> all_includes_;
    Files files_;
};

Files GetFiles(const std::string& root_path) {
    return IncludesManager().GetFiles(root_path);
}

}  // namespace karma::compiler::detail
