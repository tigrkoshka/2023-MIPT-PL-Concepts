#include "ri_executor.hpp"

#include <bit>          // for bit_cast
#include <csignal>      // for sigset_t, sigfillset, sigwait
#include <iostream>     // for cin, cout
#include <type_traits>  // for make_signed_t

#include "specs/architecture.hpp"
#include "specs/commands.hpp"

namespace karma {

namespace arch    = detail::specs::arch;
namespace cmd     = detail::specs::cmd;
namespace syscall = cmd::syscall;

arch::Word Executor::RIExecutor::ImmWord(const Args& args) {
    return static_cast<arch::Word>(args.imm);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
Executor::RIExecutor::Operation Executor::RIExecutor::HALT() {
    return [](Args) -> MaybeReturnCode {
        sigset_t wset{};
        sigfillset(&wset);

        int sig{};
        sigwait(&wset, &sig);
        return {};
    };
}

Executor::RIExecutor::Operation Executor::RIExecutor::SYSCALL() {
    return [this](Args args) -> MaybeReturnCode {
        switch (auto code = static_cast<syscall::Code>(ImmWord(args))) {
            case syscall::EXIT: {
                return RReg(args.reg);
            }

            case syscall::SCANINT: {
                std::make_signed_t<arch::Word> val{};
                std::cin >> val;

                WReg(args.reg) = static_cast<arch::Word>(val);
                break;
            }

            case syscall::SCANDOUBLE: {
                arch::Double val{};
                std::cin >> val;

                PutTwoRegisters(std::bit_cast<arch::TwoWords>(val), args.reg);
                break;
            }

            case syscall::PRINTINT: {
                using Int = std::make_signed_t<arch::Word>;
                std::cout << static_cast<Int>(RReg(args.reg)) << std::flush;
                break;
            }

            case syscall::PRINTDOUBLE: {
                arch::TwoWords words = GetTwoRegisters(args.reg);
                std::cout << std::bit_cast<arch::Double>(words) << std::flush;
                break;
            }

            case syscall::GETCHAR: {
                syscall::Char val{};
                std::cin >> val;

                WReg(args.reg) = static_cast<arch::Word>(val);
                break;
            }

            case syscall::PUTCHAR: {
                if (RReg(args.reg) > syscall::kMaxChar) {
                    throw ExecutionError::InvalidPutCharValue(RReg(args.reg));
                }

                std::cout << static_cast<syscall::Char>(RReg(args.reg))
                          << std::flush;
                break;
            }

            default: {
                throw ExecutionError::UnknownSyscallCode(code);
            }
        }

        return {};
    };
}

Executor::RIExecutor::Operation Executor::RIExecutor::ADDI() {
    return [this](Args args) -> MaybeReturnCode {
        WReg(args.reg) += ImmWord(args);
        return {};
    };
}

Executor::RIExecutor::Operation Executor::RIExecutor::SUBI() {
    return [this](Args args) -> MaybeReturnCode {
        WReg(args.reg) -= ImmWord(args);
        return {};
    };
}

Executor::RIExecutor::Operation Executor::RIExecutor::MULI() {
    return [this](Args args) -> MaybeReturnCode {
        auto res = static_cast<arch::TwoWords>(RReg(args.reg)) *
                   static_cast<arch::TwoWords>(ImmWord(args));

        PutTwoRegisters(res, args.reg);
        return {};
    };
}

Executor::RIExecutor::Operation Executor::RIExecutor::DIVI() {
    return [this](Args args) -> MaybeReturnCode {
        Divide(GetTwoRegisters(args.reg),
               static_cast<arch::TwoWords>(ImmWord(args)),
               args.reg);
        return {};
    };
}

Executor::RIExecutor::Operation Executor::RIExecutor::NOT() {
    return [this](Args args) -> MaybeReturnCode {
        WReg(args.reg) = ~RReg(args.reg);
        return {};
    };
}

Executor::RIExecutor::Operation Executor::RIExecutor::SHLI() {
    return [this](Args args) -> MaybeReturnCode {
        arch::Word rhs = ImmWord(args);
        CheckBitwiseShiftRHS(rhs, cmd::SHLI);
        WReg(args.reg) <<= rhs;
        return {};
    };
}

Executor::RIExecutor::Operation Executor::RIExecutor::SHRI() {
    return [this](Args args) -> MaybeReturnCode {
        arch::Word rhs = ImmWord(args);
        CheckBitwiseShiftRHS(rhs, cmd::SHRI);
        WReg(args.reg) >>= rhs;
        return {};
    };
}

Executor::RIExecutor::Operation Executor::RIExecutor::ANDI() {
    return [this](Args args) -> MaybeReturnCode {
        WReg(args.reg) &= ImmWord(args);
        return {};
    };
}

Executor::RIExecutor::Operation Executor::RIExecutor::ORI() {
    return [this](Args args) -> MaybeReturnCode {
        WReg(args.reg) |= ImmWord(args);
        return {};
    };
}

Executor::RIExecutor::Operation Executor::RIExecutor::XORI() {
    return [this](Args args) -> MaybeReturnCode {
        WReg(args.reg) ^= ImmWord(args);
        return {};
    };
}

Executor::RIExecutor::Operation Executor::RIExecutor::CMPI() {
    return [this](Args args) -> MaybeReturnCode {
        WriteComparisonToFlags(RReg(args.reg), ImmWord(args));
        return {};
    };
}

Executor::RIExecutor::Operation Executor::RIExecutor::PUSH() {
    return [this](Args args) -> MaybeReturnCode {
        Push(RReg(args.reg) + ImmWord(args));
        return {};
    };
}

Executor::RIExecutor::Operation Executor::RIExecutor::POP() {
    return [this](Args args) -> MaybeReturnCode {
        Pop(args.reg, ImmWord(args));
        return {};
    };
}

Executor::RIExecutor::Operation Executor::RIExecutor::LC() {
    return [this](Args args) -> MaybeReturnCode {
        WReg(args.reg) = ImmWord(args);
        return {};
    };
}

Executor::RIExecutor::Map Executor::RIExecutor::GetMap() {
    return {
        {cmd::HALT,    HALT()   },
        {cmd::SYSCALL, SYSCALL()},
        {cmd::ADDI,    ADDI()   },
        {cmd::SUBI,    SUBI()   },
        {cmd::MULI,    MULI()   },
        {cmd::DIVI,    DIVI()   },
        {cmd::NOT,     NOT()    },
        {cmd::SHLI,    SHLI()   },
        {cmd::SHRI,    SHRI()   },
        {cmd::ANDI,    ANDI()   },
        {cmd::ORI,     ORI()    },
        {cmd::XORI,    XORI()   },
        {cmd::CMPI,    CMPI()   },
        {cmd::PUSH,    PUSH()   },
        {cmd::POP,     POP()    },
        {cmd::LC,      LC()     },
    };
}

}  // namespace karma
