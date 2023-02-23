#include "specs.hpp"

namespace karma::detail::specs {

const utils::Map<CommandFormat, std::string> kFormatToString = {
    {RM, "RM"},
    {RR, "RR"},
    {RI, "RI"},
    {J,  "J" },
};

const utils::Map<CommandCode, CommandFormat> kCodeToFormat = {
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

const utils::Map<std::string, CommandCode> kCommandToCode = {
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

const utils::Map<CommandCode, std::string> kCodeToCommand =
    utils::RevertMap(kCommandToCode);

}  // namespace karma::detail::specs