#pragma once

#include <cstdint>  // for int32_t
#include <string>   // for string
#include <utility>  // for move

#include "compiler/compiler.hpp"
#include "specs/commands.hpp"
#include "specs/constants.hpp"
#include "utils/traits.hpp"

namespace karma::errors::compiler {

struct InternalError::Builder : detail::utils::traits::Static {
   private:
    using Where = const std::string&;

   public:
    static InternalError RepeatedOpenFile(const std::string& path);
    static InternalError CloseUnopenedFile(const std::string& path);

    static InternalError FormatNotFound(detail::specs::cmd::Code, Where);

    static InternalError UnprocessedCommandFormat(detail::specs::cmd::Format,
                                                  Where);

    static InternalError UnprocessedConstantType(detail::specs::consts::Type,
                                                 Where);

    static InternalError EmptyWord(Where);

    static InternalError EntrypointHasAddressButNotPos();
    static InternalError EntrypointSetAddressNoPos();
    static InternalError NoEntrypointPosInLabelSubstitution();
};

struct CompileError::Builder : detail::utils::traits::Static {
   private:
    struct TokenImpl {
        TokenImpl(std::string value, std::string where)
            : value(std::move(value)),
              where(std::move(where)) {}

        std::string value;
        std::string where;
    };

   private:
    using Where     = const std::string&;
    using Token     = const TokenImpl&;
    using Label     = Token;
    using Value     = Token;
    using Command   = Token;
    using Register  = Token;
    using Address   = Token;
    using Immediate = Token;
    using Extra     = Token;

   public:
    // file

    static CompileError FailedToOpen(Where path);

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
