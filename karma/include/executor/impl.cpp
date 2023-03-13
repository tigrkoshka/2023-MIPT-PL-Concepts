#include "impl.hpp"

#include <algorithm>    // for copy
#include <cmath>        // for floor
#include <concepts>     // for totally_ordered
#include <csignal>      // for sigset_t, sigfillset, sigwait
#include <exception>    // for exception
#include <iostream>     // for cout
#include <optional>     // for nullopt
#include <sstream>      // for ostringstream
#include <string>       // for string
#include <type_traits>  // for make_signed_t

#include "errors.hpp"
#include "exec/exec.hpp"
#include "specs/architecture.hpp"
#include "specs/flags.hpp"
#include "utils/error.hpp"
#include "utils/type_conversions.hpp"

namespace karma::executor::detail {

using errors::executor::Error;
using errors::executor::InternalError;
using errors::executor::ExecutionError;

namespace utils = karma::detail::utils;

namespace flags = karma::detail::specs::flags;
namespace arch  = karma::detail::specs::arch;

namespace cmd     = karma::detail::specs::cmd;
namespace args    = cmd::args;
namespace syscall = cmd::syscall;

namespace exec = karma::detail::exec;

////////////////////////////////////////////////////////////////////////////////
///                                   Utils                                  ///
////////////////////////////////////////////////////////////////////////////////

arch::TwoWords Impl::GetTwoRegisters(args::Source lower) const {
    return utils::types::Join(registers_[lower], registers_[lower + 1]);
}

void Impl::PutTwoRegisters(arch::TwoWords value, args::Receiver lower) {
    auto [low, high] = utils::types::Split(value);

    registers_[lower]     = low;
    registers_[lower + 1] = high;
}

void Impl::CheckBitwiseRHS(arch::Word rhs, cmd::Code code) {
    if (rhs >= sizeof(arch::Word) * utils::types::kByteSize) {
        throw ExecutionError::BitwiseRHSTooBig(rhs, code);
    }
}

template <std::totally_ordered T>
void Impl::WriteComparisonToFlags(T lhs, T rhs) {
    auto cmp_res = lhs <=> rhs;

    if (cmp_res < 0) {
        flags_ = flags::kLess;
    } else if (cmp_res > 0) {
        flags_ = flags::kGreater;
    } else {
        // condition "cmp_res == 0" is true
        // because lhs and rhs are comparable
        // because of the template type constraint
        flags_ = flags::kEqual;
    }
}

void Impl::Jump(flags::Flag flag, args::Address dst) {
    if ((flags_ & flag) != 0) {
        registers_[arch::kInstructionRegister] = dst;
    }
}

////////////////////////////////////////////////////////////////////////////////
///                        Separate reusable commands                        ///
////////////////////////////////////////////////////////////////////////////////

void Impl::Divide(arch::TwoWords lhs, arch::TwoWords rhs, args::Receiver recv) {
    if (rhs == 0) {
        throw ExecutionError::DivisionByZero(lhs, rhs);
    }

    arch::TwoWords quotient = lhs / rhs;

    if (quotient > static_cast<arch::TwoWords>(arch::kMaxWord)) {
        throw ExecutionError::QuotientOverflow(lhs, rhs);
    }

    arch::TwoWords remainder = lhs % rhs;

    registers_[recv]     = static_cast<arch::Word>(quotient);
    registers_[recv + 1] = static_cast<arch::Word>(remainder);
}

Impl::MaybeRetCode Impl::Syscall(args::Register reg, syscall::Code code) {
    switch (code) {
        case syscall::EXIT: {
            return registers_[reg];
        }

        case syscall::SCANINT: {
            std::make_signed_t<arch::Word> val{};
            std::cin >> val;

            registers_[reg] = static_cast<arch::Word>(val);
            break;
        }

        case syscall::SCANDOUBLE: {
            arch::Double val{};
            std::cin >> val;

            PutTwoRegisters(utils::types::ToUll(val), reg);
            break;
        }

        case syscall::PRINTINT: {
            std::cout << static_cast<std::make_signed_t<arch::Word>>(
                registers_[reg]);
            break;
        }

        case syscall::PRINTDOUBLE: {
            std::cout << utils::types::ToDbl(GetTwoRegisters(reg));
            break;
        }

        case syscall::GETCHAR: {
            syscall::Char val{};
            std::cin >> val;

            registers_[reg] = static_cast<arch::Word>(val);
            break;
        }

        case syscall::PUTCHAR: {
            if (registers_[reg] > syscall::kMaxChar) {
                throw ExecutionError::InvalidPutCharValue(registers_[reg]);
            }

            std::cout << static_cast<syscall::Char>(registers_[reg]);
            break;
        }

        default: {
            throw ExecutionError::UnknownSyscallCode(code);
        }
    }

    return std::nullopt;
}

void Impl::Push(arch::Word value) {
    memory_[registers_[arch::kStackRegister]--] = value;
}

void Impl::Pop(args::Receiver recv, arch::Word mod) {
    registers_[recv] = memory_[++registers_[arch::kStackRegister]] + mod;
}

arch::Address Impl::Call(args::Address callee) {
    // remember the return address to return from this function
    arch::Address ret = registers_[arch::kInstructionRegister];

    // push the return address to the stack
    Push(ret);

    // push the stack pointer before the function arguments
    Push(registers_[arch::kCallFrameRegister]);

    // store the stack pointer value before the function local variables
    registers_[arch::kCallFrameRegister] = registers_[arch::kStackRegister];

    // next executed instruction is the first one from the callee
    registers_[arch::kInstructionRegister] = callee;

    return ret;
}

void Impl::Return() {
    // move the stack to before the function local variables
    registers_[arch::kStackRegister] = registers_[arch::kCallFrameRegister];

    // pop the value of the stack pointer before the function arguments
    Pop(arch::kCallFrameRegister, 0);

    // pop the return address from the stack
    Pop(arch::kInstructionRegister, 0);

    // move the stack to before the function arguments
    registers_[arch::kStackRegister] = registers_[arch::kCallFrameRegister];

    // restore the call frame register to the stack pointer value before
    // the caller's local variables
    Pop(arch::kCallFrameRegister, 0);
}

////////////////////////////////////////////////////////////////////////////////
///                              Execute command                             ///
////////////////////////////////////////////////////////////////////////////////

Impl::MaybeRetCode Impl::ExecuteRMCommand(cmd::Code code,
                                        args::Register reg,
                                        args::Address addr) {
    switch (code) {
        case cmd::LA: {
            registers_[reg] = addr;
            break;
        }

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
            throw InternalError::UnprocessedCommandForFormat(cmd::RM, code);
        }
    }

    return std::nullopt;
}

Impl::MaybeRetCode Impl::ExecuteRRCommand(cmd::Code code,
                                        args::Receiver recv,
                                        args::Source src,
                                        args::Modifier mod) {
    auto lhs_word = [this, recv]() -> arch::Word {
        return registers_[recv];
    };

    auto lhs_dbl = [this, recv]() -> arch::Double {
        return utils::types::ToDbl(GetTwoRegisters(recv));
    };

    auto rhs_word = [this, src, mod]() -> arch::Word {
        return registers_[src] + static_cast<arch::Word>(mod);
    };

    auto rhs_dbl = [this, src]() -> arch::Double {
        return utils::types::ToDbl(GetTwoRegisters(src));
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
            auto res = static_cast<arch::TwoWords>(lhs_word()) *
                       static_cast<arch::TwoWords>(rhs_word());
            PutTwoRegisters(res, recv);
            break;
        }

        case cmd::DIV: {
            Divide(GetTwoRegisters(recv),
                   static_cast<arch::TwoWords>(rhs_word()),
                   recv);
            break;
        }

        case cmd::SHL: {
            arch::Word rhs = rhs_word();
            CheckBitwiseRHS(rhs, code);
            registers_[recv] <<= rhs;
            break;
        }

        case cmd::SHR: {
            arch::Word rhs = rhs_word();
            CheckBitwiseRHS(rhs, code);
            registers_[recv] >>= rhs;
            break;
        }

        case cmd::AND: {
            arch::Word rhs = rhs_word();
            CheckBitwiseRHS(rhs, code);
            registers_[recv] &= rhs;
            break;
        }

        case cmd::OR: {
            arch::Word rhs = rhs_word();
            CheckBitwiseRHS(rhs, code);
            registers_[recv] |= rhs;
            break;
        }

        case cmd::XOR: {
            arch::Word rhs = rhs_word();
            CheckBitwiseRHS(rhs, code);
            registers_[recv] ^= rhs;
            break;
        }

        case cmd::ITOD: {
            PutTwoRegisters(
                utils::types::ToUll(static_cast<arch::Double>(rhs_word())),
                recv);
            break;
        }

        case cmd::DTOI: {
            arch::Double dbl = rhs_dbl();
            auto res         = static_cast<arch::TwoWords>(floor(dbl));

            if (res >= static_cast<arch::TwoWords>(arch::kMaxWord)) {
                throw ExecutionError::DtoiOverflow(dbl);
            }

            registers_[recv] = static_cast<arch::Word>(res);
            break;
        }

        case cmd::ADDD: {
            PutTwoRegisters(utils::types::ToUll(lhs_dbl() + rhs_dbl()), recv);
            break;
        }

        case cmd::SUBD: {
            PutTwoRegisters(utils::types::ToUll(lhs_dbl() - rhs_dbl()), recv);
            break;
        }

        case cmd::MULD: {
            PutTwoRegisters(utils::types::ToUll(lhs_dbl() * rhs_dbl()), recv);
            break;
        }

        case cmd::DIVD: {
            arch::Double lhs = lhs_dbl();
            arch::Double rhs = rhs_dbl();

            if (rhs == 0) {
                throw ExecutionError::DivisionByZero(lhs, rhs);
            }

            PutTwoRegisters(utils::types::ToUll(lhs / rhs), recv);
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
            throw InternalError::UnprocessedCommandForFormat(cmd::RR, code);
        }
    }

    return std::nullopt;
}

Impl::MaybeRetCode Impl::ExecuteRICommand(cmd::Code code,
                                        args::Register reg,
                                        args::Immediate imm) {
    auto imm_word = [imm]() -> arch::Word {
        return static_cast<arch::Word>(imm);
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
            return Syscall(reg, static_cast<syscall::Code>(imm_word()));
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
            auto res = static_cast<arch::TwoWords>(registers_[reg]) *
                       static_cast<arch::TwoWords>(imm_word());

            PutTwoRegisters(res, reg);
            break;
        }

        case cmd::DIVI: {
            Divide(GetTwoRegisters(reg),
                   static_cast<arch::TwoWords>(imm_word()),
                   reg);
            break;
        }

        case cmd::NOT: {
            registers_[reg] = ~registers_[reg];
            break;
        }

        case cmd::SHLI: {
            arch::Word rhs = imm_word();
            CheckBitwiseRHS(rhs, code);
            registers_[reg] <<= rhs;
            break;
        }

        case cmd::SHRI: {
            arch::Word rhs = imm_word();
            CheckBitwiseRHS(rhs, code);
            registers_[reg] >>= rhs;
            break;
        }

        case cmd::ANDI: {
            arch::Word rhs = imm_word();
            CheckBitwiseRHS(rhs, code);
            registers_[reg] &= rhs;
            break;
        }

        case cmd::ORI: {
            arch::Word rhs = imm_word();
            CheckBitwiseRHS(rhs, code);
            registers_[reg] |= rhs;
            break;
        }

        case cmd::XORI: {
            arch::Word rhs = imm_word();
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
            throw InternalError::UnprocessedCommandForFormat(cmd::RI, code);
        }
    }

    return std::nullopt;
}

Impl::MaybeRetCode Impl::ExecuteJCommand(cmd::Code code, args::Address addr) {
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
            Return();
            break;
        }

        default: {
            throw InternalError::UnprocessedCommandForFormat(cmd::J, code);
        }
    }

    return std::nullopt;
}

Impl::MaybeRetCode Impl::ExecuteCommand(cmd::Bin command) {
    registers_[arch::kInstructionRegister]++;

    auto code = cmd::GetCode(command);
    if (!cmd::kCodeToFormat.contains(code)) {
        throw ExecutionError::UnknownCommand(code);
    }

    switch (cmd::Format format = cmd::kCodeToFormat.at(code)) {
        case cmd::RM: {
            auto [reg, addr] = cmd::parse::RM(command);
            return ExecuteRMCommand(code, reg, addr);
        }

        case cmd::RR: {
            auto [recv, src, mod] = cmd::parse::RR(command);
            return ExecuteRRCommand(code, recv, src, mod);
        }

        case cmd::RI: {
            auto [reg, imm] = cmd::parse::RI(command);
            return ExecuteRICommand(code, reg, imm);
        }

        case cmd::J: {
            auto [addr] = cmd::parse::J(command);
            return ExecuteJCommand(code, addr);
        }

        default: {
            throw InternalError::UnprocessedCommandFormat(format);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
///                              Execute binary                              ///
////////////////////////////////////////////////////////////////////////////////

Impl::RetCode Impl::ExecuteImpl(const std::string& exec_path) {
    exec::Data data = exec::Read(exec_path);

    registers_[arch::kCallFrameRegister]   = data.initial_stack;
    registers_[arch::kInstructionRegister] = data.entrypoint;
    registers_[arch::kStackRegister]       = data.initial_stack;

    using DiffT = std::vector<arch::Word>::iterator::difference_type;

    std::copy(data.code.begin(), data.code.end(), memory_.begin());

    std::copy(data.constants.begin(),
              data.constants.end(),
              memory_.begin() + static_cast<DiffT>(data.code.size()));

    while (true) {
        arch::Address curr_address = registers_[arch::kInstructionRegister];

        if (curr_address >= memory_.size()) {
            throw ExecutionError::ExecPointerOutOfMemory(curr_address);
        }

        cmd::Bin curr_command = memory_[curr_address];

        if (MaybeRetCode return_code = ExecuteCommand(curr_command)) {
            return *return_code;
        }
    }
}

Impl::RetCode Impl::MustExecute(const std::string& exec_path) {
    return ExecuteImpl(exec_path);
}

Impl::RetCode Impl::Execute(const std::string& exec_path) {
    try {
        return ExecuteImpl(exec_path);
    } catch (const Error& e) {
        std::cout << "executing " << exec_path << ": " << e.what() << std::endl;
        return 1;
    } catch (const errors::Error& e) {
        std::cout << exec_path << ": " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cout << exec_path << ": unexpected exception: " << e.what()
                  << std::endl;
        return 1;
    } catch (...) {
        std::cout << "executing " << exec_path << ": unexpected exception";
        return 1;
    }
}

}  // namespace karma::executor::detail
