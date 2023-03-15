#include "common_executor.hpp"

#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/flags.hpp"
#include "utils/types.hpp"

namespace karma {

namespace utils = detail::utils;
namespace arch  = detail::specs::arch;
namespace cmd   = detail::specs::cmd;
namespace args  = cmd::args;
namespace flags = detail::specs::flags;

arch::TwoWords Executor::CommonExecutor::GetTwoRegisters(args::Register low) {
    return utils::types::Join(Reg(low), Reg(low + 1));
}

void Executor::CommonExecutor::PutTwoRegisters(arch::TwoWords value,
                                               args::Receiver lower) {
    auto [low, high] = utils::types::Split(value);

    Reg(lower)     = low;
    Reg(lower + 1) = high;
}

void Executor::CommonExecutor::CheckBitwiseRHS(arch::Word rhs, cmd::Code code) {
    if (rhs >= sizeof(arch::Word) * utils::types::kByteSize) {
        throw ExecutionError::BitwiseRHSTooBig(rhs, code);
    }
}

void Executor::CommonExecutor::Divide(arch::TwoWords lhs,
                                      arch::TwoWords rhs,
                                      args::Receiver recv) {
    if (rhs == 0) {
        throw ExecutionError::DivisionByZero(lhs, rhs);
    }

    arch::TwoWords quotient = lhs / rhs;

    if (quotient > static_cast<arch::TwoWords>(arch::kMaxWord)) {
        throw ExecutionError::QuotientOverflow(lhs, rhs);
    }

    arch::TwoWords remainder = lhs % rhs;

    Reg(recv)     = static_cast<arch::Word>(quotient);
    Reg(recv + 1) = static_cast<arch::Word>(remainder);
}

void Executor::CommonExecutor::Push(arch::Word value) {
    CheckPushAllowed();
    Mem(Reg(arch::kStackRegister)--) = value;
}

void Executor::CommonExecutor::Pop(args::Receiver recv, arch::Word mod) {
    Reg(recv) = Mem(++Reg(arch::kStackRegister)) + mod;
}

void Executor::CommonExecutor::Jump(flags::Flag flag, args::Address dst) {
    if ((Flags() & flag) != 0) {
        Reg(arch::kInstructionRegister) = dst;
    }
}

arch::Address Executor::CommonExecutor::Call(args::Address callee) {
    // remember the return address to return from this function
    arch::Address ret = Reg(arch::kInstructionRegister);

    // push the return address to the stack
    Push(ret);

    // push the stack pointer before the function arguments
    Push(Reg(arch::kCallFrameRegister));

    // store the stack pointer value before the function local variables
    Reg(arch::kCallFrameRegister) = Reg(arch::kStackRegister);

    // next executed instruction is the first one from the callee
    Reg(arch::kInstructionRegister) = callee;

    return ret;
}

void Executor::CommonExecutor::Return() {
    // move the stack to before the function local variables
    Reg(arch::kStackRegister) = Reg(arch::kCallFrameRegister);

    // pop the value of the stack pointer before the function arguments
    Pop(arch::kCallFrameRegister, 0);

    // pop the return address from the stack
    Pop(arch::kInstructionRegister, 0);

    // move the stack to before the function arguments
    Reg(arch::kStackRegister) = Reg(arch::kCallFrameRegister);

    // restore the call frame register to the stack pointer value before
    // the caller's local variables
    Pop(arch::kCallFrameRegister, 0);
}

}  // namespace karma