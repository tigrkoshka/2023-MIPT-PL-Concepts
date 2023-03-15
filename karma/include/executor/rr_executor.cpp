#include "rr_executor.hpp"

#include <cmath>  // for floor

#include "executor/errors.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "utils/types.hpp"

namespace karma {

using karma::errors::executor::ExecutionError;

namespace utils = karma::detail::utils;

namespace arch = karma::detail::specs::arch;

namespace cmd  = karma::detail::specs::cmd;
namespace args = cmd::args;

arch::Word Executor::RRExecutor::LHSWord(const Args& args) {
    return Reg(args.recv);
}

arch::Double Executor::RRExecutor::LHSDouble(const Args& args) {
    return utils::types::ToDbl(GetTwoRegisters(args.recv));
}

arch::Word Executor::RRExecutor::RHSWord(const Args& args) {
    return Reg(args.src) + static_cast<arch::Word>(args.mod);
}

arch::Double Executor::RRExecutor::RHSDouble(const Args& args) {
    return utils::types::ToDbl(GetTwoRegisters(args.src));
}

Executor::RRExecutor::Operation Executor::RRExecutor::ADD() {
    return [this](Args args) -> MaybeReturnCode {
        Reg(args.recv) += RHSWord(args);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::SUB() {
    return [this](Args args) -> MaybeReturnCode {
        Reg(args.recv) -= RHSWord(args);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::MUL() {
    return [this](Args args) -> MaybeReturnCode {
        auto res = static_cast<arch::TwoWords>(LHSWord(args)) *
                   static_cast<arch::TwoWords>(RHSWord(args));
        PutTwoRegisters(res, args.recv);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::DIV() {
    return [this](Args args) -> MaybeReturnCode {
        Divide(GetTwoRegisters(args.recv),
               static_cast<arch::TwoWords>(RHSWord(args)),
               args.recv);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::SHL() {
    return [this](Args args) -> MaybeReturnCode {
        arch::Word rhs = RHSWord(args);
        CheckBitwiseRHS(rhs, cmd::SHL);
        Reg(args.recv) <<= rhs;
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::SHR() {
    return [this](Args args) -> MaybeReturnCode {
        arch::Word rhs = RHSWord(args);
        CheckBitwiseRHS(rhs, cmd::SHR);
        Reg(args.recv) >>= rhs;
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::AND() {
    return [this](Args args) -> MaybeReturnCode {
        arch::Word rhs = RHSWord(args);
        CheckBitwiseRHS(rhs, cmd::AND);
        Reg(args.recv) &= rhs;
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::OR() {
    return [this](Args args) -> MaybeReturnCode {
        arch::Word rhs = RHSWord(args);
        CheckBitwiseRHS(rhs, cmd::OR);
        Reg(args.recv) |= rhs;
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::XOR() {
    return [this](Args args) -> MaybeReturnCode {
        arch::Word rhs = RHSWord(args);
        CheckBitwiseRHS(rhs, cmd::XOR);
        Reg(args.recv) ^= rhs;
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::ITOD() {
    return [this](Args args) -> MaybeReturnCode {
        PutTwoRegisters(
            utils::types::ToUll(static_cast<arch::Double>(RHSWord(args))),
            args.recv);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::DTOI() {
    return [this](Args args) -> MaybeReturnCode {
        arch::Double dbl = RHSDouble(args);
        auto res         = static_cast<arch::TwoWords>(floor(dbl));

        if (res >= static_cast<arch::TwoWords>(arch::kMaxWord)) {
            throw ExecutionError::DtoiOverflow(dbl);
        }

        Reg(args.recv) = static_cast<arch::Word>(res);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::ADDD() {
    return [this](Args args) -> MaybeReturnCode {
        PutTwoRegisters(utils::types::ToUll(LHSDouble(args) + RHSDouble(args)),
                        args.recv);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::SUBD() {
    return [this](Args args) -> MaybeReturnCode {
        PutTwoRegisters(utils::types::ToUll(LHSDouble(args) - RHSDouble(args)),
                        args.recv);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::MULD() {
    return [this](Args args) -> MaybeReturnCode {
        PutTwoRegisters(utils::types::ToUll(LHSDouble(args) * RHSDouble(args)),
                        args.recv);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::DIVD() {
    return [this](Args args) -> MaybeReturnCode {
        arch::Double lhs = LHSDouble(args);
        arch::Double rhs = RHSDouble(args);

        if (rhs == 0) {
            throw ExecutionError::DivisionByZero(lhs, rhs);
        }

        PutTwoRegisters(utils::types::ToUll(lhs / rhs), args.recv);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::CMP() {
    return [this](Args args) -> MaybeReturnCode {
        WriteComparisonToFlags(LHSWord(args), RHSWord(args));
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::CMPD() {
    return [this](Args args) -> MaybeReturnCode {
        WriteComparisonToFlags(LHSDouble(args), RHSDouble(args));
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::MOV() {
    return [this](Args args) -> MaybeReturnCode {
        Reg(args.recv) = RHSWord(args);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::LOADR() {
    return [this](Args args) -> MaybeReturnCode {
        Reg(args.recv) = Mem(RHSWord(args));
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::LOADR2() {
    return [this](Args args) -> MaybeReturnCode {
        args::Address address = RHSWord(args);

        Reg(args.recv)     = Mem(address);
        Reg(args.recv + 1) = Mem(address + 1);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::STORER() {
    return [this](Args args) -> MaybeReturnCode {
        Mem(RHSWord(args)) = Reg(args.recv);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::STORER2() {
    return [this](Args args) -> MaybeReturnCode {
        args::Address address = RHSWord(args);

        Mem(address)     = Reg(args.recv);
        Mem(address + 1) = Reg(args.recv + 1);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::CALL() {
    return [this](Args args) -> MaybeReturnCode {
        Reg(args.recv) = Call(RHSWord(args));
        return {};
    };
}

Executor::RRExecutor::Map Executor::RRExecutor::GetMap() {
    return {
        {cmd::ADD,     ADD()    },
        {cmd::SUB,     SUB()    },
        {cmd::MUL,     MUL()    },
        {cmd::DIV,     DIV()    },
        {cmd::SHL,     SHL()    },
        {cmd::SHR,     SHR()    },
        {cmd::AND,     AND()    },
        {cmd::OR,      OR()     },
        {cmd::XOR,     XOR()    },
        {cmd::ITOD,    ITOD()   },
        {cmd::DTOI,    DTOI()   },
        {cmd::ADDD,    ADDD()   },
        {cmd::SUBD,    SUBD()   },
        {cmd::MULD,    MULD()   },
        {cmd::DIVD,    DIVD()   },
        {cmd::CMP,     CMP()    },
        {cmd::CMPD,    CMPD()   },
        {cmd::MOV,     MOV()    },
        {cmd::LOADR,   LOADR()  },
        {cmd::LOADR2,  LOADR2() },
        {cmd::STORER,  STORER() },
        {cmd::STORER2, STORER2()},
        {cmd::CALL,    CALL()   },
    };
}

}  // namespace karma
