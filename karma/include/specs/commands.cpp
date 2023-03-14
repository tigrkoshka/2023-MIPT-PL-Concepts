#include "commands.hpp"

#include <string>
#include <unordered_map>

#include "specs/architecture.hpp"
#include "utils/map.hpp"
#include "utils/types.hpp"

namespace karma::detail::specs::cmd {

constexpr arch::Word kCodeShift    = 24u;
constexpr arch::Word kRegisterMask = 0xfu;
constexpr arch::Word kRecvShift    = 20u;
constexpr arch::Word kSrcShift     = 16u;
constexpr arch::Word kAddressMask  = 0xfffffu;

Code GetCode(Bin command) {
    return static_cast<cmd::Code>(command >> kCodeShift);
}

const std::unordered_map<Format, std::string> kFormatToString = {
    {RM, "RM"},
    {RR, "RR"},
    {RI, "RI"},
    {J,  "J" },
};

const std::unordered_map<Code, Format> kCodeToFormat = {
  // System

    {HALT,    RI},
    {SYSCALL, RI},

 // Integer arithmetic

    {ADD,     RR},
    {ADDI,    RI},
    {SUB,     RR},
    {SUBI,    RI},
    {MUL,     RR},
    {MULI,    RI},
    {DIV,     RR},
    {DIVI,    RI},

 // Bitwise operators

    {NOT,     RI},
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

 // Real-valued operators

    {ITOD,    RR},
    {DTOI,    RR},
    {ADDD,    RR},
    {SUBD,    RR},
    {MULD,    RR},
    {DIVD,    RR},

 // Comparisons

    {CMP,     RR},
    {CMPI,    RI},
    {CMPD,    RR},

 // Jumps

    {JMP,     J },
    {JNE,     J },
    {JEQ,     J },
    {JLE,     J },
    {JL,      J },
    {JGE,     J },
    {JG,      J },

 // Stack

    {PUSH,    RI},
    {POP,     RI},

 // Data transfer

    {LC,      RI},
    {LA,      RM},
    {MOV,     RR},
    {LOAD,    RM},
    {LOAD2,   RM},
    {STORE,   RM},
    {STORE2,  RM},
    {LOADR,   RR},
    {LOADR2,  RR},
    {STORER,  RR},
    {STORER2, RR},

 // Function calls

    {CALL,    RR},
    {CALLI,   J },
    {RET,     J },
};

const std::unordered_map<std::string, Code> kNameToCode = {

  // System

    {"halt",    HALT   },
    {"syscall", SYSCALL},

 // Integer arithmetic

    {"add",     ADD    },
    {"addi",    ADDI   },
    {"sub",     SUB    },
    {"subi",    SUBI   },
    {"mul",     MUL    },
    {"muli",    MULI   },
    {"div",     DIV    },
    {"divi",    DIVI   },

 // Bitwise operators

    {"not",     NOT    },
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

 // Real-valued operators

    {"itod",    ITOD   },
    {"dtoi",    DTOI   },
    {"addd",    ADDD   },
    {"subd",    SUBD   },
    {"muld",    MULD   },
    {"divd",    DIVD   },

 // Comparisons

    {"cmp",     CMP    },
    {"cmpi",    CMPI   },
    {"cmpd",    CMPD   },

 // Jumps

    {"jmp",     JMP    },
    {"jne",     JNE    },
    {"jeq",     JEQ    },
    {"jle",     JLE    },
    {"jl",      JL     },
    {"jge",     JGE    },
    {"jg",      JG     },

 // Stack

    {"push",    PUSH   },
    {"pop",     POP    },

 // Data transfer

    {"lc",      LC     },
    {"la",      LA     },
    {"mov",     MOV    },
    {"load",    LOAD   },
    {"load2",   LOAD2  },
    {"store",   STORE  },
    {"store2",  STORE2 },
    {"loadr",   LOADR  },
    {"loadr2",  LOADR2 },
    {"storer",  STORER },
    {"storer2", STORER2},

 // Function calls

    {"call",    CALL   },
    {"calli",   CALLI  },
    {"ret",     RET    },
};

const std::unordered_map<Code, std::string> kCodeToName =
    utils::map::Revert(kNameToCode);

namespace parse {

args::RMArgs RM(Bin command) {
    return {
        .reg  = (command >> kRecvShift) & kRegisterMask,
        .addr = command & kAddressMask,
    };
}

args::RRArgs RR(Bin command) {
    return {
        .recv = (command >> kRecvShift) & kRegisterMask,
        .src  = (command >> kSrcShift) & kRegisterMask,
        .mod  = utils::types::GetSignedValue(command, args::kModSize),
    };
}

args::RIArgs RI(Bin command) {
    return {
        .reg = (command >> kRecvShift) & kRegisterMask,
        .imm = utils::types::GetSignedValue(command, args::kImmSize),
    };
}

args::JArgs J(Bin command) {
    return {
        .addr = command & kAddressMask,
    };
}

}  // namespace parse

namespace build {

Bin RM(Code code, args::RMArgs args) {
    return (code << kCodeShift) |      //
           (args.reg << kRecvShift) |  //
           args.addr;
}

Bin RR(Code code, args::RRArgs args) {
    return (code << kCodeShift) |       //
           (args.recv << kRecvShift) |  //
           (args.src << kSrcShift) |    //
           utils::types::GetUnsignedBits(args.mod, args::kModSize);
}

Bin RI(Code code, args::RIArgs args) {
    return (code << kCodeShift) |      //
           (args.reg << kRecvShift) |  //
           utils::types::GetUnsignedBits(args.imm, args::kImmSize);
}

Bin J(Code code, args::JArgs args) {
    return (code << kCodeShift) |  //
           args.addr;
}

}  // namespace build

}  // namespace karma::detail::specs::cmd