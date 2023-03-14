#include "ri_executor.hpp"

#include <csignal>      // for sigset_t, sigfillset, sigwait
#include <iostream>     // for cin, cout
#include <type_traits>  // for make_signed_t

#include "errors.hpp"
#include "return_code.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "utils/type_conversions.hpp"

namespace karma::executor::detail {

using karma::errors::executor::ExecutionError;

namespace utils = karma::detail::utils;

namespace arch = karma::detail::specs::arch;

namespace cmd     = karma::detail::specs::cmd;
namespace syscall = cmd::syscall;

arch::Word RIExecutor::ImmWord(const Args& args) {
    return static_cast<arch::Word>(args.imm);
}

RIExecutor::Operation RIExecutor::HALT() {
    return [](Args) -> MaybeReturnCode {
        sigset_t wset{};
        sigfillset(&wset);

        int sig{};
        sigwait(&wset, &sig);
        return {};
    };
}

RIExecutor::Operation RIExecutor::SYSCALL() {
    return [this](Args args) -> MaybeReturnCode {
        switch (auto code = static_cast<syscall::Code>(ImmWord(args))) {
            case syscall::EXIT: {
                return Reg(args.reg);
            }

            case syscall::SCANINT: {
                std::make_signed_t<arch::Word> val{};
                std::cin >> val;

                Reg(args.reg) = static_cast<arch::Word>(val);
                break;
            }

            case syscall::SCANDOUBLE: {
                arch::Double val{};
                std::cin >> val;

                PutTwoRegisters(utils::types::ToUll(val), args.reg);
                break;
            }

            case syscall::PRINTINT: {
                std::cout << static_cast<std::make_signed_t<arch::Word>>(
                    Reg(args.reg));
                break;
            }

            case syscall::PRINTDOUBLE: {
                std::cout << utils::types::ToDbl(GetTwoRegisters(args.reg));
                break;
            }

            case syscall::GETCHAR: {
                syscall::Char val{};
                std::cin >> val;

                Reg(args.reg) = static_cast<arch::Word>(val);
                break;
            }

            case syscall::PUTCHAR: {
                if (Reg(args.reg) > syscall::kMaxChar) {
                    throw ExecutionError::InvalidPutCharValue(Reg(args.reg));
                }

                std::cout << static_cast<syscall::Char>(Reg(args.reg));
                break;
            }

            default: {
                throw ExecutionError::UnknownSyscallCode(code);
            }
        }

        return {};
    };
}

RIExecutor::Operation RIExecutor::ADDI() {
    return [this](Args args) -> MaybeReturnCode {
        Reg(args.reg) += ImmWord(args);
        return {};
    };
}

RIExecutor::Operation RIExecutor::SUBI() {
    return [this](Args args) -> MaybeReturnCode {
        Reg(args.reg) -= ImmWord(args);
        return {};
    };
}

RIExecutor::Operation RIExecutor::MULI() {
    return [this](Args args) -> MaybeReturnCode {
        auto res = static_cast<arch::TwoWords>(Reg(args.reg)) *
                   static_cast<arch::TwoWords>(ImmWord(args));

        PutTwoRegisters(res, args.reg);
        return {};
    };
}

RIExecutor::Operation RIExecutor::DIVI() {
    return [this](Args args) -> MaybeReturnCode {
        Divide(GetTwoRegisters(args.reg),
               static_cast<arch::TwoWords>(ImmWord(args)),
               args.reg);
        return {};
    };
}

RIExecutor::Operation RIExecutor::NOT() {
    return [this](Args args) -> MaybeReturnCode {
        Reg(args.reg) = ~Reg(args.reg);
        return {};
    };
}

RIExecutor::Operation RIExecutor::SHLI() {
    return [this](Args args) -> MaybeReturnCode {
        arch::Word rhs = ImmWord(args);
        CheckBitwiseRHS(rhs, cmd::SHLI);
        Reg(args.reg) <<= rhs;
        return {};
    };
}

RIExecutor::Operation RIExecutor::SHRI() {
    return [this](Args args) -> MaybeReturnCode {
        arch::Word rhs = ImmWord(args);
        CheckBitwiseRHS(rhs, cmd::SHRI);
        Reg(args.reg) >>= rhs;
        return {};
    };
}

RIExecutor::Operation RIExecutor::ANDI() {
    return [this](Args args) -> MaybeReturnCode {
        arch::Word rhs = ImmWord(args);
        CheckBitwiseRHS(rhs, cmd::ANDI);
        Reg(args.reg) &= rhs;
        return {};
    };
}

RIExecutor::Operation RIExecutor::ORI() {
    return [this](Args args) -> MaybeReturnCode {
        arch::Word rhs = ImmWord(args);
        CheckBitwiseRHS(rhs, cmd::ORI);
        Reg(args.reg) |= rhs;
        return {};
    };
}

RIExecutor::Operation RIExecutor::XORI() {
    return [this](Args args) -> MaybeReturnCode {
        arch::Word rhs = ImmWord(args);
        CheckBitwiseRHS(rhs, cmd::XORI);
        Reg(args.reg) ^= rhs;
        return {};
    };
}

RIExecutor::Operation RIExecutor::CMPI() {
    return [this](Args args) -> MaybeReturnCode {
        WriteComparisonToFlags(Reg(args.reg), ImmWord(args));
        return {};
    };
}

RIExecutor::Operation RIExecutor::PUSH() {
    return [this](Args args) -> MaybeReturnCode {
        Push(Reg(args.reg) + ImmWord(args));
        return {};
    };
}

RIExecutor::Operation RIExecutor::POP() {
    return [this](Args args) -> MaybeReturnCode {
        Pop(args.reg, ImmWord(args));
        return {};
    };
}

RIExecutor::Operation RIExecutor::LC() {
    return [this](Args args) -> MaybeReturnCode {
        Reg(args.reg) = ImmWord(args);
        return {};
    };
}

RIExecutor::Map RIExecutor::GetMap() {
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

}  // namespace karma::executor::detail
