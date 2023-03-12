#pragma once

#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr, shred_ptr
#include <utility>
#include <vector>  // for vector

#include "entrypoint.hpp"
#include "exec_data.hpp"
#include "file.hpp"
#include "labels.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "utils/traits.hpp"

namespace karma::compiler::detail {

class FileCompiler : karma::detail::utils::traits::NonCopyableMovable {
   private:
    using Segment = std::vector<karma::detail::specs::arch::Word>;

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

    [[nodiscard]] karma::detail::specs::cmd::CodeFormat GetCodeFormat() const;
    [[nodiscard]] karma::detail::specs::cmd::args::Register GetRegister() const;
    [[nodiscard]] karma::detail::specs::cmd::args::Immediate GetImmediate(
        size_t size) const;
    // maybe writes to labels, so not marked const
    [[nodiscard]] karma::detail::specs::cmd::args::Address GetAddress(
        bool is_entry = false);

    karma::detail::specs::cmd::args::RMArgs GetRMArgs();
    karma::detail::specs::cmd::args::RRArgs GetRRArgs();
    karma::detail::specs::cmd::args::RIArgs GetRIArgs();
    karma::detail::specs::cmd::args::JArgs GetJArgs();

    karma::detail::specs::cmd::Bin MustParseCommand();

    void ProcessCurrLine(bool is_first_line = false);

   public:
    FileCompiler(const std::unique_ptr<File>& file,
                 std::shared_ptr<Labels> labels,
                 std::shared_ptr<Entrypoint> entrypoint)
        : file_(file),
          labels_(std::move(labels)),
          entrypoint_(std::move(entrypoint)) {}

    ExecData PrepareData() &&;

   private:
    const std::unique_ptr<File>& file_;
    const std::shared_ptr<Labels> labels_;
    const std::shared_ptr<Entrypoint> entrypoint_;

    std::string latest_label_;
    std::string latest_label_pos_;
    bool latest_word_was_label_{false};

    std::string curr_token_;

    ExecData data_;
};

}  // namespace karma::compiler::detail