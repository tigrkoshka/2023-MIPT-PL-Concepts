#pragma once

#include <cstddef>        // for size_t
#include <cstdint>        // for int32_t
#include <istream>        // for istream
#include <sstream>        // for istringstream
#include <stdexcept>      // for runtime_error
#include <string>         // for string
#include <unordered_map>  // for unordered_map
#include <utility>        // for pair
#include <vector>         // for vector

#include "specs/architecture.hpp"
#include "specs/commands.hpp"

namespace karma {

class Compiler {
   public:
    struct Error;
    struct InternalError;
    struct CompileError;

   private:
    class Impl {
       private:
        friend struct Compiler::Error;

       private:
        bool TryProcessEntrypoint();
        void CheckLabel() const;
        bool TryProcessLabel();

        detail::specs::cmd::CodeFormat GetCommand();
        detail::specs::cmd::args::Register GetRegister() const;
        detail::specs::cmd::args::Immediate GetImmediate(size_t bit_size) const;
        detail::specs::cmd::args::Address GetAddress(bool is_entry = false);

        detail::specs::cmd::args::RMArgs GetRMOperands();
        detail::specs::cmd::args::RRArgs GetRROperands();
        detail::specs::cmd::args::RIArgs GetRIOperands();
        detail::specs::cmd::args::JArgs GetJOperands();

        void ProcessCurrLine();
        void FillLabels();
        void DoCompile(std::istream& code);
        void CompileImpl(std::istream& code, const std::string& exec_path);

       public:
        void Compile(std::istream& code, const std::string& exec_path);

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
    static void Compile(std::istream& code, const std::string& exec_path);
    static void Compile(const std::string& src, const std::string& dst = "");
};

struct Compiler::Error : std::runtime_error {
   private:
    friend void Compiler::Impl::CompileImpl(std::istream& code,
                                            const std::string& exec_path);

   protected:
    explicit Error(const std::string& message)
        : std::runtime_error(message) {}

   public:
    Error(const Error&)            = default;
    Error& operator=(const Error&) = default;
    Error(Error&&)                 = default;
    Error& operator=(Error&&)      = default;
    ~Error() override              = default;
};

struct Compiler::InternalError : Error {
   private:
    friend void Compiler::Impl::Compile(std::istream& code,
                                        const std::string& exec_path);

   private:
    explicit InternalError(const std::string& message)
        : Error("internal compiler error: " + message) {}

    InternalError(const std::string& message, size_t line)
        : Error("internal compiler error at line " + std::to_string(line) +
                ": " + message) {}

   public:
    InternalError(const InternalError&)            = default;
    InternalError& operator=(const InternalError&) = default;
    InternalError(InternalError&&)                 = default;
    InternalError& operator=(InternalError&&)      = default;
    ~InternalError() override                      = default;

    static InternalError FailedToOpen(const std::string& path);

    static InternalError FormatNotFound(detail::specs::cmd::Code command_code,
                                        size_t line);

    static InternalError UnknownCommandFormat(detail::specs::cmd::Format format,
                                              size_t line);

    static InternalError EmptyWord(size_t line);
};

struct Compiler::CompileError : Error {
   private:
    explicit CompileError(const std::string& message)
        : Error("compile error: " + message) {}

    explicit CompileError(const std::string& message, size_t line)
        : Error("compile error at line " + std::to_string(line) + ": " +
                message) {}

   public:
    CompileError(const CompileError&)            = default;
    CompileError& operator=(const CompileError&) = default;
    CompileError(CompileError&&)                 = default;
    CompileError& operator=(CompileError&&)      = default;
    ~CompileError() override                     = default;

    // command

    static CompileError UnknownCommand(const std::string& command, size_t line);

    // entrypoint

    static CompileError NoEntrypoint();

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    static CompileError SecondEntrypoint(size_t line, size_t entrypoint_line);

    static CompileError EntrypointWithoutAddress(size_t line);

    // labels

    static CompileError EmptyLabel(size_t line);

    static CompileError LabelBeforeEntrypoint(size_t end_line,
                                              const std::string& latest_label,
                                              size_t latest_label_line);

    static CompileError ConsecutiveLabels(const std::string& label,
                                          size_t line,
                                          const std::string& latest_label,
                                          size_t latest_label_line);

    static CompileError FileEndsWithLabel(const std::string& label,
                                          size_t line);

    static CompileError LabelStartsWithDigit(const std::string& label,
                                             size_t line);

    static CompileError InvalidLabelCharacter(char invalid,
                                              const std::string& label,
                                              size_t line);

    static CompileError UndefinedLabel(const std::string& label, size_t line);

    // register

    static CompileError UnknownRegister(const std::string& reg, size_t line);

    // address

    static CompileError AddressNegative(const std::string& address,
                                        size_t line);

    static CompileError AddressOutOfMemory(const std::string& address,
                                           size_t line);

    // immediate

    static CompileError ImmediateNotANumber(const std::string& immediate,
                                            size_t line);

    static CompileError ImmediateLessThanMin(int32_t min,
                                             const std::string& immediate,
                                             size_t line);

    static CompileError ImmediateMoreThanMax(int32_t max,
                                             const std::string& immediate,
                                             size_t line);

    static CompileError ImmediateOutOfRange(const std::string& immediate,
                                            size_t line);

    // RM

    static CompileError RMCommandNoRegister(size_t line);

    static CompileError RMCommandNoAddress(size_t line);

    // RR

    static CompileError RRCommandNoReceiver(size_t line);

    static CompileError RRCommandNoSource(size_t line);

    static CompileError RRCommandNoModifier(size_t line);

    // RI

    static CompileError RICommandNoRegister(size_t line);

    static CompileError RICommandNoImmediate(size_t line);

    // J

    static CompileError JCommandNoAddress(size_t line);

    // Extra words

    static CompileError ExtraWordsAfterEntrypoint(const std::string& extra,
                                                  size_t line);

    static CompileError ExtraWords(detail::specs::cmd::Format format,
                                   const std::string& extra,
                                   size_t line);
};

}  // namespace karma
