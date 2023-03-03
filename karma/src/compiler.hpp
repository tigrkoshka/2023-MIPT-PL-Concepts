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

class Compiler {
   private:
    class Impl {
       private:
        bool TryProcessEntrypoint();
        void CheckLabel() const;
        bool TryProcessLabel();

        detail::specs::cmd::CodeFormat GetCommand();
        detail::specs::cmd::args::Register GetRegister() const;
        detail::specs::cmd::args::Immediate GetImmediate(size_t bit_size) const;
        detail::specs::cmd::args::Address GetAddress(bool is_entry = false);

        detail::specs::cmd::args::RMArgs GetRMArgs();
        detail::specs::cmd::args::RRArgs GetRRArgs();
        detail::specs::cmd::args::RIArgs GetRIArgs();
        detail::specs::cmd::args::JArgs GetJArgs();

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
        size_t command_number_{0};

        std::istringstream curr_line_;
        std::string curr_word_;

        size_t latest_label_line_{0};
        std::string latest_label_;
        bool latest_word_was_label_{false};

        detail::specs::arch::Address entrypoint_{0};
        std::string entrypoint_label_;
        size_t entrypoint_line_{0};
        bool seen_entrypoint_{false};

        // label -> command_number
        std::unordered_map<std::string, detail::specs::arch::Address>
            label_definitions_;

        // label -> line_number, command_number
        std::unordered_map<std::string, std::vector<std::pair<size_t, size_t>>>
            label_usages_;

        std::vector<detail::specs::arch::Word> compiled_;
    };

   public:
    static void MustCompile(std::istream& code, const std::string& exec_path);
    static void Compile(std::istream& code, const std::string& exec_path);

    static void MustCompile(const std::string& src,
                            const std::string& dst = "");
    static void Compile(const std::string& src, const std::string& dst = "");
};

}  // namespace karma
