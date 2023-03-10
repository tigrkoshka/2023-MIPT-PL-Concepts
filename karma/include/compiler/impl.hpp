#pragma once

#include <cstddef>  // for size_t
#include <istream>  // for istream
#include <sstream>  // for istringstream
#include <string>   // for string
#include <vector>   // for vector

#include "labels.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"

namespace karma::compiler::detail {

class Impl {
   private:
    size_t CurrCmdAddress();
    size_t CurrConstAddress();

    bool TryProcessLabel();
    bool TryProcessEntrypoint();

    void ProcessUint32Constant();
    void ProcessUint64Constant();
    void ProcessDoubleConstant();
    void ProcessCharConstant();
    void ProcessStringConstant();
    bool TryProcessConstant();

    karma::detail::specs::cmd::CodeFormat GetCodeFormat() const;
    karma::detail::specs::cmd::args::Register GetRegister() const;
    karma::detail::specs::cmd::args::Immediate GetImmediate(size_t size) const;
    karma::detail::specs::cmd::args::Address GetAddress(bool is_entry = false);

    karma::detail::specs::cmd::args::RMArgs GetRMArgs();
    karma::detail::specs::cmd::args::RRArgs GetRRArgs();
    karma::detail::specs::cmd::args::RIArgs GetRIArgs();
    karma::detail::specs::cmd::args::JArgs GetJArgs();

    karma::detail::specs::cmd::Bin MustParseCommand();

    static size_t FindCommentStart(const std::string& line);
    void ProcessCurrLine();

    void FillLabels();

    void PrepareExecData(std::istream& code);

    void CompileImpl(std::istream& code, const std::string& exec_path);
    void CompileImpl(const std::string& src, const std::string& dst);

   public:
    void MustCompile(std::istream& code, const std::string& exec_path);
    void Compile(std::istream& code, const std::string& exec_path);

    void MustCompile(const std::string& src, const std::string& dst);
    void Compile(const std::string& src, const std::string& dst);

   private:
    size_t line_number_{1ull};

    std::istringstream curr_line_;
    std::string curr_word_;

    bool latest_word_was_label_{false};

    karma::detail::specs::arch::Address entrypoint_{0u};
    size_t entrypoint_line_{0ull};
    bool seen_entrypoint_{false};

    std::vector<karma::detail::specs::arch::Word> code_;
    std::vector<karma::detail::specs::arch::Word> constants_;

    Labels labels_;
};

}  // namespace karma::compiler::detail
