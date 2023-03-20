#pragma once

#include <cstddef>     // for size_t
#include <filesystem>  // for path
#include <fstream>     // for ifstream
#include <sstream>     // for istringstream
#include <string>      // for string

#include "compiler/compiler.hpp"
#include "compiler/errors.hpp"
#include "utils/generator.hpp"
#include "utils/traits.hpp"

namespace karma {

class Compiler::File : detail::utils::traits::NonCopyableMovable {
   private:
    using InternalError = errors::compiler::InternalError::Builder;
    using CompileError  = errors::compiler::CompileError::Builder;

   private:
    static void TrimComment(std::string& line);

    [[nodiscard]] detail::utils::Generator<const File*> ToRoot() const;

   public:
    explicit File(std::filesystem::path path, const File* parent = nullptr)
        : path_(std::move(path)),
          parent_(parent) {}

    void Open();

    void Close();

    bool NextLine();

    bool GetLine(std::string& line);

    bool GetToken(std::string& token);

    [[nodiscard]] std::string WhereNoLine() const;

    [[nodiscard]] std::string Where() const;

    [[nodiscard]] size_t LineNum() const;

    [[nodiscard]] const std::filesystem::path& Path() const;

   private:
    static const std::string kIncludedFromSep;

    const std::filesystem::path path_;
    const File* parent_;

    std::ifstream stream_;

    size_t line_{0};
    std::istringstream curr_line_;
};

}  // namespace karma
