#include "storage.hpp"

#include "exec/exec.hpp"
#include "specs/architecture.hpp"
#include "utils/vector.hpp"

namespace karma::executor::detail {

namespace utils = karma::detail::utils;

namespace arch = karma::detail::specs::arch;

namespace exec = karma::detail::exec;

void Storage::PrepareForExecution(const exec::Data& exec_data) {
    utils::vector::CopyToBegin(memory_, exec_data.code, exec_data.constants);

    registers_[arch::kCallFrameRegister]   = exec_data.initial_stack;
    registers_[arch::kStackRegister]       = exec_data.initial_stack;
    registers_[arch::kInstructionRegister] = exec_data.entrypoint;
}

arch::Word& Storage::Reg(arch::Register reg) {
    return registers_[reg];
}

arch::Word& Storage::Mem(arch::Address address) {
    return registers_[address];
}

arch::Word& Storage::Flags() {
    return flags_;
}

}  // namespace karma::executor::detail
