#pragma once

#include <cstddef>        // for size_t
#include <cstdint>        // for int32_t
#include <istream>        // for istream
#include <sstream>        // for istringstream
#include <string>         // for string
#include <unordered_map>  // for unordered_map
#include <utility>        // for pair
#include <vector>         // for vector

#include "specs/architecture.hpp"
#include "specs/commands.hpp"

namespace karma {

namespace detail::compiler {
class Impl {
   private:
    struct Labels {
       private:
        // definition, definition_line
        using Definition = std::pair<size_t, size_t>;

        // label -> definition
        using Definitions = std::unordered_map<std::string, Definition>;

        // line_number, position in code of the command which uses the label
        using Usages = std::vector<std::pair<size_t, size_t>>;

        // label -> LabelUsage
        using AllUsages = std::unordered_map<std::string, Usages>;

       public:
        static void CheckLabel(const std::string& label, size_t line);

        void SetCodeSize(size_t code_size);

        const std::string& GetLatest() const;
        size_t GetLatestLine() const;

        std::optional<size_t> TryGetDefinition(const std::string& label) const;
        std::optional<size_t> TryGetDefinitionLine(const std::string& label) const;

        void RecordLabelOccurrence(const std::string& label, size_t line);
        void RecordCommandLabel(size_t definition);
        void RecordConstantLabel(size_t definition);

        void RecordEntrypointLabel(const std::string& label);
        const std::optional<std::string>& TryGetEntrypointLabel();

        void RecordUsage(const std::string& label,
                         size_t line_number,
                         size_t command_number);
        const AllUsages& GetUsages();

       private:
        std::string latest_label_;
        size_t latest_label_line_{0};

        Definitions commands_labels_;
        Definitions constants_labels_;
        std::optional<std::string> entrypoint_label_;

        AllUsages usages_;

        size_t code_size_{};
    };

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

    detail::specs::cmd::CodeFormat GetCodeFormat() const;
    detail::specs::cmd::args::Register GetRegister() const;
    detail::specs::cmd::args::Immediate GetImmediate(size_t bit_size) const;
    detail::specs::cmd::args::Address GetAddress(bool is_entrypoint = false);

    detail::specs::cmd::args::RMArgs GetRMArgs();
    detail::specs::cmd::args::RRArgs GetRRArgs();
    detail::specs::cmd::args::RIArgs GetRIArgs();
    detail::specs::cmd::args::JArgs GetJArgs();

    detail::specs::cmd::Bin MustParseCommand();

    size_t FindCommentStart(const std::string& line) const;
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
    size_t line_number_{0};

    std::istringstream curr_line_;
    std::string curr_word_;

    bool latest_word_was_label_{false};

    detail::specs::arch::Address entrypoint_{0};
    size_t entrypoint_line_{0};
    bool seen_entrypoint_{false};

    std::vector<detail::specs::arch::types::Word> code_;
    std::vector<detail::specs::arch::types::Word> constants_;

    Labels labels_;
};

}  // namespace detail::compiler

void MustCompile(std::istream& code, const std::string& exec_path);
void Compile(std::istream& code, const std::string& exec_path);

void MustCompile(const std::string& src, const std::string& dst = "");
void Compile(const std::string& src, const std::string& dst = "");

}  // namespace karma
