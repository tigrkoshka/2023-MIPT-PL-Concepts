#pragma once

#include "architecture.hpp"

namespace karma::detail::specs::cmd {

using Bin = arch::Word;

enum Format : arch::Word {
    RM = 0,
    RR = 1,
    RI = 2,
    J  = 3,
};

enum Code : arch::Word {
    HALT    = 0,
    SYSCALL = 1,
    ADD     = 2,
    ADDI    = 3,
    SUB     = 4,
    SUBI    = 5,
    MUL     = 6,
    MULI    = 7,
    DIV     = 8,
    DIVI    = 9,
    LC      = 12,
    SHL     = 13,
    SHLI    = 14,
    SHR     = 15,
    SHRI    = 16,
    AND     = 17,
    ANDI    = 18,
    OR      = 19,
    ORI     = 20,
    XOR     = 21,
    XORI    = 22,
    NOT     = 23,
    MOV     = 24,
    ADDD    = 32,
    SUBD    = 33,
    MULD    = 34,
    DIVD    = 35,
    ITOD    = 36,
    DTOI    = 37,
    PUSH    = 38,
    POP     = 39,
    CALL    = 40,
    CALLI   = 41,
    RET     = 42,
    CMP     = 43,
    CMPI    = 44,
    CMPD    = 45,
    JMP     = 46,
    JNE     = 47,
    JEQ     = 48,
    JLE     = 49,
    JL      = 50,
    JGE     = 51,
    JG      = 52,
    LOAD    = 64,
    STORE   = 65,
    LOAD2   = 66,
    STORE2  = 67,
    LOADR   = 68,
    LOADR2  = 69,
    STORER  = 70,
    STORER2 = 71,
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
using Modifier  = arch::Word;
using Immediate = arch::Word;

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