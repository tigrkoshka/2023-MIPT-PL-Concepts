#pragma once

#include <cstddef>  // for size_t
#include <cstdint>  // for int32_t
#include <string>   // for string, to_string

#include "../specs/commands.hpp"
#include "error.hpp"

namespace karma::errors::compiler {

struct Error : errors::Error {
   protected:
    explicit Error(const std::string& message)
        : errors::Error(message) {}
};

struct InternalError : Error {
   private:
    explicit InternalError(const std::string& message)
        : Error("internal compiler error: " + message) {}

    InternalError(const std::string& message, size_t line)
        : Error("internal compiler error at line " + std::to_string(line) +
                ": " + message) {}

   public:
    static InternalError FailedToOpen(const std::string& path);

    static InternalError FormatNotFound(detail::specs::cmd::Code command_code,
                                        size_t line);

    static InternalError UnknownCommandFormat(detail::specs::cmd::Format format,
                                              size_t line);

    static InternalError EmptyWord(size_t line);
};

struct CompileError : Error {
   private:
    explicit CompileError(const std::string& message)
        : Error("compile error: " + message) {}

    explicit CompileError(const std::string& message, size_t line)
        : Error("compile error at line " + std::to_string(line) + ": " +
                message) {}

   public:
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

}  // namespace karma::errors::compiler