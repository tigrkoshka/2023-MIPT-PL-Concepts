#pragma once

#include <limits>         // for numeric_limits
#include <string>         // for string
#include <type_traits>    // for make_signed_t
#include <unordered_map>  // for unordered_map

#include "specs/architecture.hpp"

namespace karma::detail::specs::cmd {

////////////////////////////////////////////////////////////////////////////////
///                              Code and format                             ///
////////////////////////////////////////////////////////////////////////////////

using Bin = arch::Word;

enum Format : arch::Word {
    RM,
    RR,
    RI,
    J,
};

enum Code : Bin {
    // System

    HALT,
    SYSCALL,

    // Integer arithmetic

    ADD,
    ADDI,
    SUB,
    SUBI,
    MUL,
    MULI,
    DIV,
    DIVI,

    // Bitwise operators

    NOT,
    SHL,
    SHLI,
    SHR,
    SHRI,
    AND,
    ANDI,
    OR,
    ORI,
    XOR,
    XORI,

    // Real-valued operators

    ITOD,
    DTOI,
    ADDD,
    SUBD,
    MULD,
    DIVD,

    // Comparisons

    CMP,
    CMPI,
    CMPD,

    // Jumps

    JMP,
    JNE,
    JEQ,
    JLE,
    JL,
    JGE,
    JG,

    // Stack

    PUSH,
    POP,

    // Data transfer

    LC,
    LA,
    MOV,
    LOAD,
    LOAD2,
    STORE,
    STORE2,
    LOADR,
    LOADR2,
    STORER,
    STORER2,

    // Function calls

    PRC,
    CALL,
    CALLI,
    RET,
};

struct CodeFormat {
    Code code;
    Format format;
};

Code GetCode(Bin);

extern const std::unordered_map<Format, std::string> kFormatToString;
extern const std::unordered_map<Code, Format> kCodeToFormat;
extern const std::unordered_map<std::string, Code> kNameToCode;
extern const std::unordered_map<Code, std::string> kCodeToName;

////////////////////////////////////////////////////////////////////////////////
///                                   Args                                   ///
////////////////////////////////////////////////////////////////////////////////

namespace args {

using Register  = arch::Register;
using Receiver  = arch::Register;
using Source    = arch::Register;
using Address   = arch::Address;
using Modifier  = std::make_signed_t<arch::Word>;
using Immediate = std::make_signed_t<arch::Word>;

constexpr arch::Word kModSize = 16;
constexpr arch::Word kImmSize = 20;

struct RMArgs {
    Register reg;
    Address addr;
};

struct RRArgs {
    Receiver recv;
    Source src;
    Modifier mod;
};

struct RIArgs {
    Register reg;
    Immediate imm;
};

struct JArgs {
    Address addr;
};

}  // namespace args

////////////////////////////////////////////////////////////////////////////////
///                                  Syscall                                 ///
////////////////////////////////////////////////////////////////////////////////

namespace syscall {

using Char = unsigned char;

constexpr Char kMaxChar = std::numeric_limits<Char>::max();

enum Code : cmd::args::Immediate {
    EXIT        = 0,
    SCANINT     = 100,
    SCANDOUBLE  = 101,
    PRINTINT    = 102,
    PRINTDOUBLE = 103,
    GETCHAR     = 104,
    PUTCHAR     = 105,
};

}  // namespace syscall

////////////////////////////////////////////////////////////////////////////////
///                                   Parse                                  ///
////////////////////////////////////////////////////////////////////////////////

namespace parse {

args::RMArgs RM(Bin);
args::RRArgs RR(Bin);
args::RIArgs RI(Bin);
args::JArgs J(Bin);

}  // namespace parse

////////////////////////////////////////////////////////////////////////////////
///                                   Build                                  ///
////////////////////////////////////////////////////////////////////////////////

namespace build {

Bin RM(Code, args::RMArgs);
Bin RR(Code, args::RRArgs);
Bin RI(Code, args::RIArgs);
Bin J(Code, args::JArgs);

}  // namespace build

}  // namespace karma::detail::specs::cmd
