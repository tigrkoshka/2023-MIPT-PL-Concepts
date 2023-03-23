#pragma once

#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr
#include <string>   // for string
#include <vector>   // for vector

#include "compiler/compiler.hpp"
#include "compiler/errors.hpp"
#include "compiler/file_data.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "utils/traits.hpp"

namespace karma {

class Compiler::FileCompiler : detail::utils::traits::NonCopyableMovable {
   private:
    using InternalError = errors::compiler::InternalError::Builder;
    using CompileError  = errors::compiler::CompileError::Builder;

    using Segment = std::vector<detail::specs::arch::Word>;

   private:
    [[nodiscard]] std::string Where() const;

    void SkipIncludes();

    bool TryProcessLabel();
    bool TryProcessEntrypoint();

    void ProcessUint32Constant();
    void ProcessUint64Constant();
    void ProcessDoubleConstant();
    void ProcessCharConstant();
    void ProcessStringConstant();
    bool TryProcessConstant();

    [[nodiscard]] detail::specs::cmd::CodeFormat GetCodeFormat() const;
    [[nodiscard]] detail::specs::cmd::args::Register GetRegister() const;
    [[nodiscard]] detail::specs::cmd::args::Immediate GetImmediate(
        size_t size) const;
    // maybe writes to labels, so not marked const
    [[nodiscard]] detail::specs::cmd::args::Address GetAddress(
        bool is_entry = false);

    detail::specs::cmd::args::RMArgs GetRMArgs();
    detail::specs::cmd::args::RRArgs GetRRArgs();
    detail::specs::cmd::args::RIArgs GetRIArgs();
    detail::specs::cmd::args::JArgs GetJArgs();

    detail::specs::cmd::Bin MustParseCommand();

    void ProcessCurrLine(bool is_first_line = false);

   public:
    // delete the default constructor defined in NonCopyableMovable
    FileCompiler() = delete;

    explicit FileCompiler(const std::unique_ptr<File>& file)
        : file_(file) {}

    FileData PrepareData() &&;

   private:
    const std::unique_ptr<File>& file_;
    std::string curr_token_;

    std::string latest_label_;
    std::string latest_label_pos_;
    bool latest_word_was_label_{false};

    FileData data_;
};

}  // namespace karma
