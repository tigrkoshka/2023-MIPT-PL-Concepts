#pragma once

#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>

#include "utils.hpp"

namespace karma::detail::specs {

// TODO: divide this namespace

// TODO: maybe remove typedefs from here (semantically incorrect)

////////////////////////////////////////////////////////////////////////////////
///                        Architecture-defined typedefs                     ///
////////////////////////////////////////////////////////////////////////////////

using Word     = uint32_t;
using TwoWords = uint64_t;
using Double   = double;  // TODO: std::float64_t in C++23 (supported by GCC@13)
using Char     = unsigned char;

const size_t kWordSize     = sizeof(Word);
const Word kMaxWord        = std::numeric_limits<Word>::max();
static const Char kMaxChar = std::numeric_limits<Char>::max();

////////////////////////////////////////////////////////////////////////////////
///                                  Syntax                                  ///
////////////////////////////////////////////////////////////////////////////////

static const char8_t kCommentSep = ';';
static const char8_t kLabelEnd   = ':';

////////////////////////////////////////////////////////////////////////////////
///                           Registers and memory                           ///
////////////////////////////////////////////////////////////////////////////////

using Register = Word;

enum RegisterEnum : Register {
    R0  = 0,
    R1  = 1,
    R2  = 2,
    R3  = 3,
    R4  = 4,
    R5  = 5,
    R6  = 6,
    R7  = 7,
    R8  = 8,
    R9  = 9,
    R10 = 10,
    R11 = 11,
    R12 = 12,
    R13 = 13,
    R14 = 14,
    R15 = 15,
};

static const Register kCallFrameRegister   = R13;
static const Register kStackRegister       = R14;
static const Register kInstructionRegister = R15;

extern const std::unordered_map<std::string, Register> kRegisterToNum;

static const size_t kNRegisters = 16ull;
static const size_t kMemorySize = 1ull << 20ull;

////////////////////////////////////////////////////////////////////////////////
///                                 Commands                                 ///
////////////////////////////////////////////////////////////////////////////////

enum CommandCode : Word {
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

enum CommandFormat : Word {
    RM = 0,
    RR = 1,
    RI = 2,
    J  = 3,
};

enum SyscallCode : Word {
    EXIT        = 0,
    SCANINT     = 100,
    SCANDOUBLE  = 101,
    PRINTINT    = 102,
    PRINTDOUBLE = 103,
    GETCHAR     = 104,
    PUTCHAR     = 105,
};

extern const std::unordered_map<CommandFormat, std::string> kFormatToString;
extern const std::unordered_map<CommandCode, CommandFormat> kCodeToFormat;

extern const std::unordered_map<std::string, CommandCode> kCommandToCode;
extern const std::unordered_map<CommandCode, std::string> kCodeToCommand;

////////////////////////////////////////////////////////////////////////////////
///                          Command format typedefs                         ///
////////////////////////////////////////////////////////////////////////////////

using CommandBin = Word;

using Receiver  = Register;
using Source    = Register;
using Address   = Word;
using Modifier  = Word;
using Immediate = Word;

////////////////////////////////////////////////////////////////////////////////
///                                   Flags                                  ///
////////////////////////////////////////////////////////////////////////////////

enum Flag : Word {
    EQUAL            = 0b1u,
    NOT_EQUAL        = 0b10u,
    GREATER          = 0b100u,
    LESS             = 0b1000u,
    GREATER_OR_EQUAL = 0b10000u,
    LESS_OR_EQUAL    = 0b100000u,
};

static const Word kEqual   = EQUAL + GREATER_OR_EQUAL + LESS_OR_EQUAL;
static const Word kGreater = NOT_EQUAL + GREATER + GREATER_OR_EQUAL;
static const Word kLess    = NOT_EQUAL + LESS + LESS_OR_EQUAL;

////////////////////////////////////////////////////////////////////////////////
///                             Command structure                            ///
////////////////////////////////////////////////////////////////////////////////

namespace cmd {

const Word kCodeShift    = 24u;
const Word kRegisterMask = 0xfu;
const Word kRecvShift    = 20u;
const Word kSrcShift     = 16u;
const Word kAddressMask  = 0xfffffu;
const Word kModSize      = 16u;
const Word kModMask      = 0xffffu;
const Word kImmSize      = 20u;
const Word kImmMask      = 0xfffffu;

}  // namespace cmd

////////////////////////////////////////////////////////////////////////////////
///                                Exec format                               ///
////////////////////////////////////////////////////////////////////////////////

namespace exec {

const std::string kIntroString       = "ThisIsKarmaExec";
const size_t kIntroSize              = kIntroString.size() + 1;
const std::streamsize kHeaderSize    = 512ll;
const std::streampos kCodeSegmentPos = kHeaderSize;

}  // namespace exec

}  // namespace karma::detail::specs
