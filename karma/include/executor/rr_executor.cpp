#include "rr_executor.hpp"

#include <bit>    // for bit_cast
#include <cmath>  // for floor

#include "specs/architecture.hpp"
#include "specs/commands.hpp"

namespace karma {

namespace arch = detail::specs::arch;
namespace cmd  = detail::specs::cmd;
namespace args = cmd::args;

arch::Word Executor::RRExecutor::LHSWord(const Args& args) {
    return RReg(args.recv);
}

arch::Double Executor::RRExecutor::LHSDouble(const Args& args) {
    return std::bit_cast<arch::Double>(GetTwoRegisters(args.recv));
}

arch::Word Executor::RRExecutor::RHSWord(const Args& args) {
    return RReg(args.src) + static_cast<arch::Word>(args.mod);
}

arch::Double Executor::RRExecutor::RHSDouble(const Args& args) {
    return std::bit_cast<arch::Double>(GetTwoRegisters(args.src));
}

Executor::RRExecutor::Operation Executor::RRExecutor::ADD() {
    return [this](Args args) -> MaybeReturnCode {
        WReg(args.recv) += RHSWord(args);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::SUB() {
    return [this](Args args) -> MaybeReturnCode {
        WReg(args.recv) -= RHSWord(args);
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
        const arch::Word rhs = RHSWord(args);
        CheckBitwiseShiftRHS(rhs, cmd::SHL);
        WReg(args.recv) <<= rhs;
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::SHR() {
    return [this](Args args) -> MaybeReturnCode {
        const arch::Word rhs = RHSWord(args);
        CheckBitwiseShiftRHS(rhs, cmd::SHR);
        WReg(args.recv) >>= rhs;
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::AND() {
    return [this](Args args) -> MaybeReturnCode {
        WReg(args.recv) &= RHSWord(args);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::OR() {
    return [this](Args args) -> MaybeReturnCode {
        WReg(args.recv) |= RHSWord(args);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::XOR() {
    return [this](Args args) -> MaybeReturnCode {
        WReg(args.recv) ^= RHSWord(args);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::ITOD() {
    return [this](Args args) -> MaybeReturnCode {
        auto value = static_cast<arch::Double>(RHSWord(args));
        PutTwoRegisters(std::bit_cast<arch::TwoWords>(value), args.recv);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::DTOI() {
    return [this](Args args) -> MaybeReturnCode {
        const arch::Double dbl = RHSDouble(args);
        if (dbl >= static_cast<arch::Double>(arch::kMaxWord)) {
            throw ExecutionError::DtoiOverflow(dbl);
        }

        // static cast does not produce UB, because the resulting
        // value fits into arch::Word due to the check above
        WReg(args.recv) = static_cast<arch::Word>(floor(dbl));
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::ADDD() {
    return [this](Args args) -> MaybeReturnCode {
        const arch::Double res = LHSDouble(args) + RHSDouble(args);
        PutTwoRegisters(std::bit_cast<arch::TwoWords>(res), args.recv);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::SUBD() {
    return [this](Args args) -> MaybeReturnCode {
        const arch::Double res = LHSDouble(args) - RHSDouble(args);
        PutTwoRegisters(std::bit_cast<arch::TwoWords>(res), args.recv);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::MULD() {
    return [this](Args args) -> MaybeReturnCode {
        const arch::Double res = LHSDouble(args) * RHSDouble(args);
        PutTwoRegisters(std::bit_cast<arch::TwoWords>(res), args.recv);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::DIVD() {
    return [this](Args args) -> MaybeReturnCode {
        const arch::Double lhs = LHSDouble(args);
        const arch::Double rhs = RHSDouble(args);

        if (rhs == 0) {
            throw ExecutionError::DivisionByZero(lhs, rhs);
        }

        PutTwoRegisters(std::bit_cast<arch::TwoWords>(lhs / rhs), args.recv);
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
        WReg(args.recv) = RHSWord(args);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::LOADR() {
    return [this](Args args) -> MaybeReturnCode {
        WReg(args.recv) = RMem(RHSWord(args));
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::LOADR2() {
    return [this](Args args) -> MaybeReturnCode {
        const args::Address address = RHSWord(args);

        WReg(args.recv)     = RMem(address);
        WReg(args.recv + 1) = RMem(address + 1);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::STORER() {
    return [this](Args args) -> MaybeReturnCode {
        WMem(RHSWord(args)) = RReg(args.recv);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::STORER2() {
    return [this](Args args) -> MaybeReturnCode {
        const args::Address address = RHSWord(args);

        WMem(address)     = RReg(args.recv);
        WMem(address + 1) = RReg(args.recv + 1);
        return {};
    };
}

Executor::RRExecutor::Operation Executor::RRExecutor::CALL() {
    return [this](Args args) -> MaybeReturnCode {
        WReg(args.recv) = Call(RHSWord(args));
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
