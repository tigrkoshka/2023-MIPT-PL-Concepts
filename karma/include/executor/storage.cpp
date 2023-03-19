#include "storage.hpp"

#include <ostream>  // for ostream

#include "exec/exec.hpp"
#include "executor/config.hpp"
#include "specs/architecture.hpp"
#include "utils/vector.hpp"

namespace karma {

namespace utils = detail::utils;
namespace arch  = detail::specs::arch;

void Executor::Storage::PrepareForExecution(const Exec::Data& exec_data,
                                            const Config& config,
                                            std::ostream& log) {
    curr_config_ = base_config_ & config;

    log << "[executor]: current execution config:" << std::endl
        << curr_config_ << std::endl;

    utils::vector::CopyToBegin(memory_, exec_data.code, exec_data.constants);
    curr_code_end_      = exec_data.code.size();
    curr_constants_end_ = curr_code_end_ + exec_data.constants.size();

    registers_.at(arch::kCallFrameRegister)   = exec_data.initial_stack;
    registers_.at(arch::kStackRegister)       = exec_data.initial_stack;
    registers_.at(arch::kInstructionRegister) = exec_data.entrypoint;
}

void Executor::Storage::CheckPushAllowed() {
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

arch::Word Executor::Storage::RReg(arch::Register reg,
                                   bool internal_usage) const {
    if (reg >= arch::kNRegisters) {
        throw ExecutionError::InvalidRegister(reg);
    }

    if (!internal_usage && curr_config_.RegisterIsReadWriteBlocked(reg)) {
        throw ExecutionError::RegisterIsBlocked(reg);
    }

    return registers_.at(reg);
}

arch::Word Executor::Storage::RMem(arch::Address address,
                                   bool internal_usage) const {
    if (address >= arch::kMemorySize) {
        throw ExecutionError::AddressOutOfMemory(address);
    }

    if (!internal_usage && curr_config_.CodeSegmentIsReadWriteBlocked() &&
        address < curr_code_end_) {
        throw ExecutionError::CodeSegmentBlocked(address);
    }

    if (!internal_usage && curr_config_.ConstantsSegmentIsReadWriteBlocked() &&
        address >= curr_code_end_ && address < curr_constants_end_) {
        throw ExecutionError::ConstantsSegmentBlocked(address);
    }

    return memory_.at(address);
}

arch::Word& Executor::Storage::WReg(arch::Register reg, bool internal_usage) {
    if (reg >= arch::kNRegisters) {
        throw ExecutionError::InvalidRegister(reg);
    }

    if (!internal_usage && curr_config_.RegisterIsWriteBlocked(reg)) {
        throw ExecutionError::RegisterIsBlocked(reg);
    }

    return registers_.at(reg);
}

arch::Word& Executor::Storage::WMem(arch::Address address,
                                    bool internal_usage) {
    if (address >= arch::kMemorySize) {
        throw ExecutionError::AddressOutOfMemory(address);
    }

    if (!internal_usage && curr_config_.CodeSegmentIsWriteBlocked() &&
        address < curr_code_end_) {
        throw ExecutionError::CodeSegmentBlocked(address);
    }

    if (!internal_usage && curr_config_.CodeSegmentIsWriteBlocked() &&
        address >= curr_code_end_ && address < curr_constants_end_) {
        throw ExecutionError::ConstantsSegmentBlocked(address);
    }

    return memory_.at(address);
}

arch::Word& Executor::Storage::Flags() {
    return flags_;
}

}  // namespace karma
