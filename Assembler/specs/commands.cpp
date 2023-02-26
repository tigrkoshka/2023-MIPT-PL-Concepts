#include "commands.hpp"

#include <string>
#include <unordered_map>

#include "../utils.hpp"

namespace karma::detail::specs::cmd {

const arch::Word kCodeShift    = 24u;
const arch::Word kRegisterMask = 0xfu;
const arch::Word kRecvShift    = 20u;
const arch::Word kSrcShift     = 16u;
const arch::Word kAddressMask  = 0xfffffu;
const arch::Word kModMask      = 0xffffu;
const arch::Word kImmMask      = 0xfffffu;

Code GetCode(Bin command) {
    return static_cast<cmd::Code>(command >> kCodeShift);
}

namespace parse {

args::RMArgs RM(Bin command) {
    args::Register reg = (command >> kRecvShift) & kRegisterMask;
    args::Address addr = command & kAddressMask;

    return {reg, addr};
}

args::RRArgs RR(Bin command) {
    args::Receiver recv = (command >> kRecvShift) & kRegisterMask;
    args::Source src    = (command >> kSrcShift) & kRegisterMask;
    args::Modifier mod  = command & kModMask;

    return {recv, src, mod};
}

args::RIArgs RI(Bin command) {
    args::Register reg  = (command >> kRecvShift) & kRegisterMask;
    args::Immediate imm = command & kImmMask;

    return {reg, imm};
}

args::JArgs J(Bin command) {
    args::Address addr = command & kAddressMask;

    return {addr};
}

}  // namespace parse

namespace build {

Bin RM(Code code, args::RMArgs args) {
    auto [reg, addr] = args;

    return (code << kCodeShift) |  //
           (reg << kRecvShift) |   //
           addr;
}

Bin RR(Code code, args::RRArgs args) {
    auto [recv, src, mod] = args;

    return (code << kCodeShift) |  //
           (recv << kRecvShift) |  //
           (src << kSrcShift) |    //
           mod;
}

Bin RI(Code code, args::RIArgs args) {
    auto [recv, imm] = args;

    return (code << kCodeShift) |  //
           (recv << kRecvShift) |  //
           imm;
}

Bin J(Code code, args::JArgs args) {
    auto [addr] = args;

    return (code << kCodeShift) |  //
           addr;
}

}  // namespace build

const std::unordered_map<Format, std::string> kFormatToString = {
    {RM, "RM"},
    {RR, "RR"},
    {RI, "RI"},
    {J,  "J" },
};

const std::unordered_map<Code, Format> kCodeToFormat = {
    {HALT,    RI},
    {SYSCALL, RI},
    {ADD,     RR},
    {ADDI,    RI},
    {SUB,     RR},
    {SUBI,    RI},
    {MUL,     RR},
    {MULI,    RI},
    {DIV,     RR},
    {DIVI,    RI},
    {LC,      RI},
    {SHL,     RR},
    {SHLI,    RI},
    {SHR,     RR},
    {SHRI,    RI},
    {AND,     RR},
    {ANDI,    RI},
    {OR,      RR},
    {ORI,     RI},
    {XOR,     RR},
    {XORI,    RI},
    {NOT,     RI},
    {MOV,     RR},
    {ADDD,    RR},
    {SUBD,    RR},
    {MULD,    RR},
    {DIVD,    RR},
    {ITOD,    RR},
    {DTOI,    RR},
    {PUSH,    RI},
    {POP,     RI},
    {CALL,    RR},
    {CALLI,   J },
    {RET,     J },
    {CMP,     RR},
    {CMPI,    RI},
    {CMPD,    RR},
    {JMP,     J },
    {JNE,     J },
    {JEQ,     J },
    {JLE,     J },
    {JL,      J },
    {JGE,     J },
    {JG,      J },
    {LOAD,    RM},
    {STORE,   RM},
    {LOAD2,   RM},
    {STORE2,  RM},
    {LOADR,   RR},
    {STORER,  RR},
    {LOADR2,  RR},
    {STORER2, RR},
};

const std::unordered_map<std::string, Code> kNameToCode = {
    {"halt",    HALT   },
    {"syscall", SYSCALL},
    {"add",     ADD    },
    {"addi",    ADDI   },
    {"sub",     SUB    },
    {"subi",    SUBI   },
    {"mul",     MUL    },
    {"muli",    MULI   },
    {"div",     DIV    },
    {"divi",    DIVI   },
    {"lc",      LC     },
    {"shl",     SHL    },
    {"shli",    SHLI   },
    {"shr",     SHR    },
    {"shri",    SHRI   },
    {"and",     AND    },
    {"andi",    ANDI   },
    {"or",      OR     },
    {"ori",     ORI    },
    {"xor",     XOR    },
    {"xori",    XORI   },
    {"not",     NOT    },
    {"mov",     MOV    },
    {"addd",    ADDD   },
    {"subd",    SUBD   },
    {"muld",    MULD   },
    {"divd",    DIVD   },
    {"itod",    ITOD   },
    {"dtoi",    DTOI   },
    {"push",    PUSH   },
    {"pop",     POP    },
    {"call",    CALL   },
    {"calli",   CALLI  },
    {"ret",     RET    },
    {"cmp",     CMP    },
    {"cmpi",    CMPI   },
    {"cmpd",    CMPD   },
    {"jmp",     JMP    },
    {"jne",     JNE    },
    {"jeq",     JEQ    },
    {"jle",     JLE    },
    {"jl",      JL     },
    {"jge",     JGE    },
    {"jg",      JG     },
    {"load",    LOAD   },
    {"store",   STORE  },
    {"load2",   LOAD2  },
    {"store2",  STORE2 },
    {"loadr",   LOADR  },
    {"storer",  STORER },
    {"loadr2",  LOADR2 },
    {"storer2", STORER2},
};

const std::unordered_map<Code, std::string> kCodeToName =
    util::RevertMap(kNameToCode);

}  // namespace karma::detail::specs::cmd