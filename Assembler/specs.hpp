#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "utils.hpp"

namespace karma::detail::specs {

enum CommandCode : uint8_t {
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

enum CommandFormat : uint8_t {
    RM = 0,
    RR = 1,
    RI = 2,
    J  = 3,
};

enum SyscallCode : uint32_t {
    EXIT        = 0,
    SCANINT     = 100,
    SCANDOUBLE  = 101,
    PRINTINT    = 102,
    PRINTDOUBLE = 103,
    GETCHAR     = 104,
    PUTCHAR     = 105,
};

extern const utils::Map<CommandFormat, std::string> kFormatToString;
extern const utils::Map<CommandCode, CommandFormat> kCodeToFormat;
extern const utils::Map<std::string, CommandCode> kCommandToCode;
extern const utils::Map<CommandCode, std::string> kCodeToCommand;

using Word               = uint32_t;
const uint32_t kWordSize = 4u;

const uint32_t kCommandCodeShift = 24u;
const uint32_t kRegisterMask     = 0xfu;
const uint32_t kRecvShift        = 20u;
const uint32_t kSrcShift         = 16u;
const uint32_t kAddressMask      = 0xfffffu;
const uint32_t kModSize          = 16u;
const uint32_t kModMask          = 0xffffu;
const uint32_t kImmSize          = 20u;
const uint32_t kImmMask          = 0xfffffu;

namespace exec {

const std::string kIntroString = "ThisIsKarmaExec";
const auto kIntroSize =
    static_cast<std::streamsize>(kIntroString.size()) + 1;  // including '\0'
const std::streampos kCodeSizePos      = kIntroSize;
const std::streampos kConstantsSizePos = kCodeSizePos + 4ll;
const std::streampos kDataSizePos      = kConstantsSizePos + 4ll;
const std::streampos kEntrypointPos    = kDataSizePos + 4ll;
const std::streampos kInitialStackPos  = kEntrypointPos + 4ll;

const std::streamsize kHeaderSize    = 512ll;
const std::streampos kCodeSegmentPos = kHeaderSize;

}  // namespace exec

}  // namespace karma::detail::specs
