#include "storage.hpp"

#include "config.hpp"
#include "errors.hpp"
#include "exec/exec.hpp"
#include "specs/architecture.hpp"
#include "utils/vector.hpp"

namespace karma::executor::detail {

using errors::executor::ExecutionError;

namespace utils = karma::detail::utils;

namespace arch = karma::detail::specs::arch;

namespace exec = karma::detail::exec;

void Storage::PrepareForExecution(const exec::Data& exec_data,
                                  const Config& config) {
    curr_config_ = base_config_ & config;

    utils::vector::CopyToBegin(memory_, exec_data.code, exec_data.constants);
    curr_code_end_      = exec_data.code.size();
    curr_constants_end_ = curr_code_end_ + exec_data.constants.size();

    registers_.at(arch::kCallFrameRegister)   = exec_data.initial_stack;
    registers_.at(arch::kStackRegister)       = exec_data.initial_stack;
    registers_.at(arch::kInstructionRegister) = exec_data.entrypoint;
}

void Storage::CheckPushAllowed() {
    arch::Address curr_stack_address = registers_.at(arch::kStackRegister);

    if (curr_stack_address > arch::kMemorySize) {
        // precaution in case the stack is unbounded, but has somehow
        // rewritten the constants and the code and still trying to push
        throw ExecutionError::StackPointerOutOfMemory(curr_stack_address);
    }

    if (curr_stack_address < curr_config_.MinStackAddress()) {
        throw ExecutionError::StackOverflow(curr_config_.MaxStackSize());
    }
}

arch::Word& Storage::Reg(arch::Register reg) {
    if (reg >= arch::kNRegisters) {
        throw ExecutionError::InvalidRegister(reg);
    }

    if (curr_config_.RegisterIsBlocked(reg)) {
        throw ExecutionError::RegisterIsBlocked(reg);
    }

    return registers_.at(reg);
}

arch::Word& Storage::Mem(arch::Address address) {
    if (address >= arch::kMemorySize) {
        throw ExecutionError::AddressOutOfMemory(address);
    }

    if (curr_config_.CodeSegmentIsBlocked() && address < curr_code_end_) {
        throw ExecutionError::CodeSegmentBlocked(address);
    }

    if (curr_config_.ConstantsSegmentIsBlocked() &&  //
        address >= curr_code_end_ &&                 //
        address < curr_constants_end_) {
        throw ExecutionError::ConstantsSegmentBlocked(address);
    }

    return memory_.at(address);
}

arch::Word& Storage::Flags() {
    return flags_;
}

}  // namespace karma::executor::detail
