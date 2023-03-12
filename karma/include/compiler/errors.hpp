#pragma once

#include <cstddef>  // for size_t
#include <cstdint>  // for int32_t
#include <string>   // for string, to_string
#include <utility>  // for exchange

#include "labels.hpp"
#include "specs/commands.hpp"
#include "specs/constants.hpp"
#include "utils/error.hpp"

namespace karma::errors::compiler {

struct Error : errors::Error {
   protected:
    explicit Error(const std::string& message)
        : errors::Error(message) {}
};

struct InternalError : Error {
    // TODO: maybe like in compile error

   private:
    using Where = const std::string&;

   private:
    explicit InternalError(const std::string& message)
        : Error("internal compiler error: " + message) {}

    InternalError(const std::string& message, Where where)
        : Error("internal compiler error " + where + ": " + message) {}

   public:
    static InternalError RepeatedOpenFile(const std::string& path);
    static InternalError CloseUnopenedFile(const std::string& path);

    // TODO: this is not an internal error
    static InternalError FailedToOpen(const std::string& path);

    static InternalError FormatNotFound(detail::specs::cmd::Code, Where);

    static InternalError UnprocessedCommandFormat(detail::specs::cmd::Format,
                                                  Where);

    static InternalError UnprocessedConstantType(detail::specs::consts::Type,
                                                 Where);

    static InternalError EmptyWord(Where);
};

struct CompileError : Error {
    // TODO: maybe major rethink

   private:
    struct TokenInfo {
       public:
        TokenInfo(const std::string& value, const std::string& where)
            : value_(std::move(value)),
              where_(std::move(where)) {}

        std::string Value() {
            if (std::exchange(value_used_, true)) {
                throw std::runtime_error("token value reuse in compile error");
            }

            return std::move(value_);
        }

        std::string Where() {
            if (std::exchange(where_used_, true)) {
                throw std::runtime_error("token where reuse in compile error");
            }

            return std::move(where_);
        }

       private:
        std::string value_;
        bool value_used_{false};

        std::string where_;
        bool where_used_{false};
    };

   private:
    using Where     = const std::string&;
    using Token     = TokenInfo&&;
    using Label     = Token;
    using Value     = Token;
    using Command   = Token;
    using Register  = Token;
    using Address   = Token;
    using Immediate = Token;
    using Extra     = Token;

   private:
    explicit CompileError(const std::string& message)
        : Error("compile error: " + message) {}

    CompileError(const std::string& message, Where where)
        : Error("compile error " + static_cast<std::string>(where) + ": " +
                message) {}

   public:
    // includes

    static CompileError IncludeNoFilename(Where);

    // labels

    static CompileError EmptyLabel(Where);
    static CompileError LabelBeforeEntrypoint(Where entry, Label label);
    static CompileError ConsecutiveLabels(Label curr, Label prev);
    static CompileError LabelRedefinition(Label label, Where previous_pos);
    static CompileError FileEndsWithLabel(Label label);
    static CompileError LabelStartsWithDigit(Label label);
    static CompileError InvalidLabelCharacter(char invalid, Label label);
    static CompileError UndefinedLabel(Label label);

    // entrypoint

    static CompileError NoEntrypoint();
    static CompileError SecondEntrypoint(Where curr, Where prev);
    static CompileError EntrypointWithoutAddress(Where);

    // constants

    static CompileError EmptyConstValue(detail::specs::consts::Type, Where);
    static CompileError InvalidConstValue(detail::specs::consts::Type, Value);

    static CompileError CharTooSmallForQuotes(Value);
    static CompileError CharNoStartQuote(Value);
    static CompileError CharNoEndQuote(Value);

    static CompileError StringTooSmallForQuotes(Value);
    static CompileError StringNoStartQuote(Value);
    static CompileError StringNoEndQuote(Value);

    // arguments parsing

    static CompileError UnknownCommand(Command);

    static CompileError UnknownRegister(Register);

    static CompileError AddressNegative(Address);
    static CompileError AddressOutOfMemory(Address);

    static CompileError ImmediateNotANumber(Immediate);
    static CompileError ImmediateLessThanMin(int32_t min, Immediate);
    static CompileError ImmediateMoreThanMax(int32_t max, Immediate);
    static CompileError ImmediateOutOfRange(Immediate);

    // missing arguments

    static CompileError RMNoRegister(Where);
    static CompileError RMNoAddress(Where);

    static CompileError RRNoReceiver(Where);
    static CompileError RRNoSource(Where);
    static CompileError RRNoModifier(Where);

    static CompileError RINoRegister(Where);
    static CompileError RINoImmediate(Where);

    static CompileError JNoAddress(Where);

    // extra words

    static CompileError ExtraAfterEntrypoint(Extra);
    static CompileError ExtraAfterConstant(detail::specs::consts::Type, Extra);
    static CompileError ExtraAfterCommand(detail::specs::cmd::Format, Extra);
};

}  // namespace karma::errors::compiler