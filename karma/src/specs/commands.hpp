#pragma once

#include <string>         // for string
#include <tuple>          // for tuple
#include <type_traits>    // for make_signed_t
#include <unordered_map>  // for unordered_map

#include "architecture.hpp"

namespace karma::detail::specs::cmd {

using Bin = arch::Word;

enum Format : arch::Word {
    RM,
    RR,
    RI,
    J,
};

enum Code : arch::Word {
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

    // Real-valued arithmetic

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

    CALL,
    CALLI,
    RET,
};

using CodeFormat = std::tuple<Code, Format>;

Code GetCode(Bin);

namespace syscall {

enum Code : arch::Word {
    EXIT        = 0,
    SCANINT     = 100,
    SCANDOUBLE  = 101,
    PRINTINT    = 102,
    PRINTDOUBLE = 103,
    GETCHAR     = 104,
    PUTCHAR     = 105,
};

}  // namespace syscall

namespace args {

using Register  = arch::Register;
using Receiver  = arch::Register;
using Source    = arch::Register;
using Address   = arch::Address;
using Modifier  = std::make_signed_t<arch::Word>;
using Immediate = std::make_signed_t<arch::Word>;

const arch::Word kModSize = 16u;
const arch::Word kImmSize = 20u;

using RMArgs = std::tuple<arch::Register, Address>;
using RRArgs = std::tuple<Receiver, Source, Modifier>;
using RIArgs = std::tuple<arch::Register, Immediate>;
using JArgs  = std::tuple<Address>;

}  // namespace args

namespace parse {

args::RMArgs RM(Bin);
args::RRArgs RR(Bin);
args::RIArgs RI(Bin);
args::JArgs J(Bin);

}  // namespace parse

namespace build {

Bin RM(Code, args::RMArgs);
Bin RR(Code, args::RRArgs);
Bin RI(Code, args::RIArgs);
Bin J(Code, args::JArgs);

}  // namespace build

extern const std::unordered_map<Format, std::string> kFormatToString;
extern const std::unordered_map<Code, Format> kCodeToFormat;
extern const std::unordered_map<std::string, Code> kNameToCode;
extern const std::unordered_map<Code, std::string> kCodeToName;

}  // namespace karma::detail::specs::cmd