#include "executor.hpp"

#include <algorithm>    // for copy
#include <cmath>        // for floor
#include <concepts>     // for totally_ordered
#include <csignal>      // for sigset_t, sigfillset, sigwait
#include <exception>    // for exception
#include <iostream>     // for cout
#include <sstream>      // for ostringstream
#include <type_traits>  // for make_signed_t

#include "errors/executor_errors.hpp"
#include "specs/architecture.hpp"
#include "specs/exec.hpp"
#include "specs/flags.hpp"
#include "utils/types.hpp"

namespace karma {

using errors::executor::Error;
using errors::executor::InternalError;
using errors::executor::ExecutionError;

namespace utils = detail::utils;

namespace flags = detail::specs::flags;
namespace arch  = detail::specs::arch;
namespace types = arch::types;

namespace cmd     = detail::specs::cmd;
namespace args    = cmd::args;
namespace syscall = cmd::syscall;

namespace exec = detail::specs::exec;

////////////////////////////////////////////////////////////////////////////////
///                                   Utils                                  ///
////////////////////////////////////////////////////////////////////////////////

types::Double Executor::ToDbl(types::TwoWords ull) {
    return *reinterpret_cast<types::Double*>(&ull);
}

types::TwoWords Executor::ToUll(types::Double dbl) {
    return *reinterpret_cast<types::TwoWords*>(&dbl);
}

types::TwoWords Executor::GetTwoRegisters(args::Source lower) const {
    return (static_cast<types::TwoWords>(registers_[lower + 1])
            << types::kWordSize) +
           static_cast<types::TwoWords>(registers_[lower]);
}

void Executor::PutTwoRegisters(types::TwoWords value, args::Receiver lower) {
    registers_[lower]     = static_cast<types::Word>(value);
    registers_[lower + 1] = static_cast<types::Word>(value >> types::kWordSize);
}

void Executor::CheckBitwiseRHS(types::Word rhs, cmd::Code code) {
    if (rhs >= sizeof(types::Word) * utils::types::kByteSize) {
        throw ExecutionError::BitwiseRHSTooBig(rhs, code);
    }
}

template <std::totally_ordered T>
void Executor::WriteComparisonToFlags(T lhs, T rhs) {
    auto cmp_res = lhs <=> rhs;

    if (cmp_res == 0) {
        flags_ = flags::kEqual;
    } else if (cmp_res > 0) {
        flags_ = flags::kGreater;
    } else {
        // condition "cmp_res < 0" is true
        // because lhs and rhs are comparable
        // because of the template type constraint
        flags_ = flags::kLess;
    }
}

void Executor::Jump(flags::Flag flag, args::Address dst) {
    if ((flags_ & flag) != 0) {
        registers_[arch::kInstructionRegister] = dst;
    }
}

////////////////////////////////////////////////////////////////////////////////
///                             Separate commands                            ///
////////////////////////////////////////////////////////////////////////////////

void Executor::Divide(types::TwoWords lhs,
                      types::TwoWords rhs,
                      args::Receiver recv) {
    if (rhs == 0) {
        throw ExecutionError::DivisionByZero(lhs, rhs);
    }

    types::TwoWords quotient = lhs / rhs;

    if (quotient > static_cast<types::TwoWords>(types::kMaxWord)) {
        throw ExecutionError::QuotientOverflow(lhs, rhs);
    }

    types::TwoWords remainder = lhs % rhs;

    registers_[recv]     = static_cast<types::Word>(quotient);
    registers_[recv + 1] = static_cast<types::Word>(remainder);
}

bool Executor::Syscall(args::Register reg, syscall::Code code) {
    switch (code) {
        case syscall::EXIT: {
            return false;
        }

        case syscall::SCANINT: {
            std::make_signed_t<types::Word> val{};
            std::cin >> val;

            registers_[reg] = static_cast<types::Word>(val);
            break;
        }

        case syscall::SCANDOUBLE: {
            types::Double val{};
            std::cin >> val;

            PutTwoRegisters(ToUll(val), reg);
            break;
        }

        case syscall::PRINTINT: {
            std::cout << static_cast<std::make_signed_t<types::Word>>(
                registers_[reg]);
            break;
        }

        case syscall::PRINTDOUBLE: {
            std::cout << ToDbl(GetTwoRegisters(reg));
            break;
        }

        case syscall::GETCHAR: {
            types::Char val{};
            std::cin >> val;

            registers_[reg] = static_cast<types::Word>(val);
            break;
        }

        case syscall::PUTCHAR: {
            if (registers_[reg] > types::kMaxChar) {
                throw ExecutionError::InvalidPutCharValue(registers_[reg]);
            }

            std::cout << static_cast<types::Char>(registers_[reg]);
            break;
        }

        default: {
            throw ExecutionError::UnknownSyscallCode(code);
        }
    }

    return true;
}

void Executor::Push(types::Word value) {
    memory_[registers_[arch::kStackRegister]--] = value;
}

void Executor::Pop(args::Receiver recv, types::Word mod) {
    registers_[recv] = memory_[++registers_[arch::kStackRegister]] + mod;
}

arch::Address Executor::Call(args::Address callee) {
    arch::Address ret = registers_[arch::kInstructionRegister];

    Push(ret);
    registers_[arch::kInstructionRegister] = callee;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
///                              Execute command                             ///
////////////////////////////////////////////////////////////////////////////////

bool Executor::ExecuteRMCommand(cmd::Code code, args::RMArgs args) {
    auto [reg, addr] = args;

    switch (code) {
        case cmd::LOAD: {
            registers_[reg] = memory_[addr];
            break;
        }

        case cmd::LOAD2: {
            registers_[reg]     = memory_[addr];
            registers_[reg + 1] = memory_[addr + 1];
            break;
        }

        case cmd::STORE: {
            memory_[addr] = registers_[reg];
            break;
        }

        case cmd::STORE2: {
            memory_[addr]     = registers_[reg];
            memory_[addr + 1] = registers_[reg + 1];
            break;
        }

        default: {
            throw InternalError::UnknownCommandForFormat(cmd::RM, code);
        }
    }

    return true;
}

bool Executor::ExecuteRRCommand(cmd::Code code, args::RRArgs args) {
    auto [recv, src, mod] = args;

    auto lhs_word = [this, recv]() -> types::Word {
        return registers_[recv];
    };

    auto lhs_dbl = [this, recv]() -> types::Double {
        return ToDbl(GetTwoRegisters(recv));
    };

    auto rhs_word = [this, src, mod]() -> types::Word {
        return registers_[src] + static_cast<types::Word>(mod);
    };

    auto rhs_dbl = [this, src]() -> types::Double {
        return ToDbl(GetTwoRegisters(src));
    };

    switch (code) {
        case cmd::ADD: {
            registers_[recv] += rhs_word();
            break;
        }

        case cmd::SUB: {
            registers_[recv] -= rhs_word();
            break;
        }

        case cmd::MUL: {
            types::TwoWords res = static_cast<types::TwoWords>(lhs_word()) *
                                  static_cast<types::TwoWords>(rhs_word());
            PutTwoRegisters(res, recv);
            break;
        }

        case cmd::DIV: {
            Divide(GetTwoRegisters(recv),
                   static_cast<types::TwoWords>(rhs_word()),
                   recv);
            break;
        }

        case cmd::SHL: {
            types::Word rhs = rhs_word();
            CheckBitwiseRHS(rhs, code);
            registers_[recv] <<= rhs;
            break;
        }

        case cmd::SHR: {
            types::Word rhs = rhs_word();
            CheckBitwiseRHS(rhs, code);
            registers_[recv] >>= rhs;
            break;
        }

        case cmd::AND: {
            types::Word rhs = rhs_word();
            CheckBitwiseRHS(rhs, code);
            registers_[recv] &= rhs;
            break;
        }

        case cmd::OR: {
            types::Word rhs = rhs_word();
            CheckBitwiseRHS(rhs, code);
            registers_[recv] |= rhs;
            break;
        }

        case cmd::XOR: {
            types::Word rhs = rhs_word();
            CheckBitwiseRHS(rhs, code);
            registers_[recv] ^= rhs;
            break;
        }

        case cmd::ITOD: {
            PutTwoRegisters(ToUll(static_cast<types::Double>(rhs_word())),
                            recv);
            break;
        }

        case cmd::DTOI: {
            types::Double dbl = rhs_dbl();
            auto res          = static_cast<types::TwoWords>(floor(dbl));

            if (res >= static_cast<types::TwoWords>(types::kMaxWord)) {
                throw ExecutionError::DtoiOverflow(dbl);
            }

            registers_[recv] = static_cast<types::Word>(res);
            break;
        }

        case cmd::ADDD: {
            PutTwoRegisters(ToUll(lhs_dbl() + rhs_dbl()), recv);
            break;
        }

        case cmd::SUBD: {
            PutTwoRegisters(ToUll(lhs_dbl() - rhs_dbl()), recv);
            break;
        }

        case cmd::MULD: {
            PutTwoRegisters(ToUll(lhs_dbl() * rhs_dbl()), recv);
            break;
        }

        case cmd::DIVD: {
            types::Double lhs = lhs_dbl();
            types::Double rhs = rhs_dbl();

            if (rhs == 0) {
                throw ExecutionError::DivisionByZero(lhs, rhs);
            }

            PutTwoRegisters(ToUll(lhs / rhs), recv);
            break;
        }

        case cmd::CMP: {
            WriteComparisonToFlags(lhs_word(), rhs_word());
            break;
        }

        case cmd::CMPD: {
            WriteComparisonToFlags(lhs_dbl(), rhs_dbl());
            break;
        }

        case cmd::MOV: {
            registers_[recv] = rhs_word();
            break;
        }

        case cmd::LOADR: {
            registers_[recv] = memory_[rhs_word()];
            break;
        }

        case cmd::LOADR2: {
            args::Address address = rhs_word();

            registers_[recv]     = memory_[address];
            registers_[recv + 1] = memory_[address + 1];
            break;
        }

        case cmd::STORER: {
            memory_[rhs_word()] = registers_[recv];
            break;
        }

        case cmd::STORER2: {
            args::Address address = rhs_word();

            memory_[address]     = registers_[recv];
            memory_[address + 1] = registers_[recv + 1];
            break;
        }

        case cmd::CALL: {
            registers_[recv] = Call(rhs_word());
            break;
        }

        default: {
            throw InternalError::UnknownCommandForFormat(cmd::RR, code);
        }
    }

    return true;
}

bool Executor::ExecuteRICommand(cmd::Code code, args::RIArgs args) {
    auto [reg, imm] = args;

    auto imm_word = [imm]() -> types::Word {
        return static_cast<types::Word>(imm);
    };

    switch (code) {
        case cmd::HALT: {
            sigset_t wset{};
            sigfillset(&wset);

            int sig{};
            sigwait(&wset, &sig);

            break;
        }

        case cmd::SYSCALL: {
            if (!Syscall(reg, static_cast<syscall::Code>(imm_word()))) {
                return false;
            }

            break;
        }

        case cmd::ADDI: {
            registers_[reg] += imm_word();
            break;
        }

        case cmd::SUBI: {
            registers_[reg] -= imm_word();
            break;
        }

        case cmd::MULI: {
            auto res = static_cast<types::TwoWords>(registers_[reg]) *
                       static_cast<types::TwoWords>(imm_word());

            PutTwoRegisters(res, reg);
            break;
        }

        case cmd::DIVI: {
            Divide(GetTwoRegisters(reg),
                   static_cast<types::TwoWords>(imm_word()),
                   reg);
            break;
        }

        case cmd::NOT: {
            registers_[reg] = ~registers_[reg];
            break;
        }

        case cmd::SHLI: {
            types::Word rhs = imm_word();
            CheckBitwiseRHS(rhs, code);
            registers_[reg] <<= rhs;
            break;
        }

        case cmd::SHRI: {
            types::Word rhs = imm_word();
            CheckBitwiseRHS(rhs, code);
            registers_[reg] >>= rhs;
            break;
        }

        case cmd::ANDI: {
            types::Word rhs = imm_word();
            CheckBitwiseRHS(rhs, code);
            registers_[reg] &= rhs;
            break;
        }

        case cmd::ORI: {
            types::Word rhs = imm_word();
            CheckBitwiseRHS(rhs, code);
            registers_[reg] |= rhs;
            break;
        }

        case cmd::XORI: {
            types::Word rhs = imm_word();
            CheckBitwiseRHS(rhs, code);
            registers_[reg] ^= rhs;
            break;
        }

        case cmd::CMPI: {
            WriteComparisonToFlags(registers_[reg], imm_word());
            break;
        }

        case cmd::PUSH: {
            Push(registers_[reg] + imm_word());
            break;
        }

        case cmd::POP: {
            Pop(reg, imm_word());
            break;
        }

        case cmd::LC: {
            registers_[reg] = imm_word();
            break;
        }

        default: {
            throw InternalError::UnknownCommandForFormat(cmd::RI, code);
        }
    }

    return true;
}

bool Executor::ExecuteJCommand(cmd::Code code, args::JArgs args) {
    auto [addr] = args;

    switch (code) {
        case cmd::JMP: {
            registers_[arch::kInstructionRegister] = addr;
            break;
        }

        case cmd::JNE: {
            Jump(flags::NOT_EQUAL, addr);
            break;
        }

        case cmd::JEQ: {
            Jump(flags::EQUAL, addr);
            break;
        }

        case cmd::JLE: {
            Jump(flags::LESS_OR_EQUAL, addr);
            break;
        }

        case cmd::JL: {
            Jump(flags::LESS, addr);
            break;
        }

        case cmd::JGE: {
            Jump(flags::GREATER_OR_EQUAL, addr);
            break;
        }

        case cmd::JG: {
            Jump(flags::GREATER, addr);
            break;
        }

        case cmd::CALLI: {
            Call(addr);
            break;
        }

        case cmd::RET: {
            Pop(arch::kInstructionRegister, 0);
            registers_[arch::kStackRegister] += addr;
            break;
        }

        default: {
            throw InternalError::UnknownCommandForFormat(cmd::J, code);
        }
    }

    return true;
}

bool Executor::ExecuteCommand(cmd::Bin command) {
    registers_[arch::kInstructionRegister]++;

    auto code = cmd::GetCode(command);
    if (!cmd::kCodeToFormat.contains(code)) {
        throw ExecutionError::UnknownCommand(code);
    }

    switch (cmd::Format format = cmd::kCodeToFormat.at(code)) {
        case cmd::RM: {
            return ExecuteRMCommand(code, cmd::parse::RM(command));
        }

        case cmd::RR: {
            return ExecuteRRCommand(code, cmd::parse::RR(command));
        }

        case cmd::RI: {
            return ExecuteRICommand(code, cmd::parse::RI(command));
        }

        case cmd::J: {
            return ExecuteJCommand(code, cmd::parse::J(command));
        }

        default: {
            throw InternalError::UnknownCommandFormat(format);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
///                              Execute binary                              ///
////////////////////////////////////////////////////////////////////////////////

void Executor::ExecuteImpl(const std::string& exec_path) {
    exec::Data data = exec::Read(exec_path);

    registers_[arch::kInstructionRegister] = data.entrypoint;
    registers_[arch::kStackRegister]       = data.initial_stack;

    std::copy(data.code.begin(), data.code.end(), memory_.begin());

    while (registers_[arch::kInstructionRegister] < data.code.size() &&
           ExecuteCommand(memory_[registers_[arch::kInstructionRegister]])) {
    }
}

void Executor::MustExecute(const std::string& exec_path) {
    ExecuteImpl(exec_path);
}

void Executor::Execute(const std::string& exec_path) {
    try {
        ExecuteImpl(exec_path);
    } catch (const Error& e) {
        std::cout << "executing " << exec_path << ": " << e.what() << std::endl;
    } catch (const errors::Error& e) {
        std::cout << exec_path << ": " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << exec_path << ": unexpected exception: " << e.what()
                  << std::endl;
    } catch (...) {
        std::cout << "executing " << exec_path << ": unexpected exception";
    }
}

}  // namespace karma
