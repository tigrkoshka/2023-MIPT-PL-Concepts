#pragma once

#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr, shred_ptr
#include <string>   // for string
#include <utility>  // for move
#include <vector>   // for vector

#include "compiler/compiler.hpp"
#include "compiler/errors.hpp"
#include "compiler/exec_data.hpp"
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

    Segment& Code();
    [[nodiscard]] const Segment& Code() const;

    Segment& Constants();
    [[nodiscard]] const Segment& Constants() const;

    void SkipIncludes();

    [[nodiscard]] size_t CurrCmdAddress() const;
    [[nodiscard]] size_t CurrConstAddress() const;

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

    FileCompiler(const std::unique_ptr<File>& file,
                 std::shared_ptr<Labels> labels,
                 std::shared_ptr<Entrypoint> entrypoint,
                 size_t prev_code_size,
                 size_t prev_constants_size)
        : file_(file),
          labels_(std::move(labels)),
          entrypoint_(std::move(entrypoint)),
          prev_code_size_(prev_code_size),
          prev_constants_size_(prev_constants_size) {}

    ExecData PrepareData() &&;

   private:
    const std::unique_ptr<File>& file_;
    const std::shared_ptr<Labels> labels_;
    const std::shared_ptr<Entrypoint> entrypoint_;

    std::string latest_label_;
    std::string latest_label_pos_;
    bool latest_word_was_label_{false};

    std::string curr_token_;

    size_t prev_code_size_{0};
    size_t prev_constants_size_{0};
    ExecData data_;
};

}  // namespace karma