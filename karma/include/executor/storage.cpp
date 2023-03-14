#include "storage.hpp"

#include <cstddef>   // for size_t
#include <optional>  // for optional

#include "config.hpp"
#include "exec/exec.hpp"
#include "specs/architecture.hpp"
#include "utils/vector.hpp"

namespace karma::executor::detail {

namespace utils = karma::detail::utils;

namespace arch = karma::detail::specs::arch;

namespace exec = karma::detail::exec;

void Storage::PrepareForExecution(const exec::Data& exec_data,
                                  const Config& config) {
    curr_config_ = base_config_ & config;
    if (std::optional<size_t> max_stack_size = curr_config_.MaxStackSize()) {
        curr_min_stack_allowed_ = arch::kMemorySize - *max_stack_size;
    }

    utils::vector::CopyToBegin(memory_, exec_data.code, exec_data.constants);
    curr_code_end_      = exec_data.code.size();
    curr_constants_end_ = curr_code_end_ + exec_data.constants.size();

    registers_.at(arch::kCallFrameRegister)   = exec_data.initial_stack;
    registers_.at(arch::kStackRegister)       = exec_data.initial_stack;
    registers_.at(arch::kInstructionRegister) = exec_data.entrypoint;
}

void Storage::CheckPushAllowed() {
    if (registers_.at(arch::kStackRegister) > arch::kMemorySize) {
        // precaution in case the stack is unbounded, but has somehow
        // rewritten the constants and the code and still trying to push
        // TODO: throw
    }

    if (registers_.at(arch::kStackRegister) < curr_min_stack_allowed_) {
        // TODO: throw
    }
}

arch::Word& Storage::Reg(arch::Register reg) {
    if (reg >= arch::kNRegisters) {
        // TODO: throw
    }

    if (curr_config_.RegisterIsBlocked(reg)) {
        // TODO: throw
    }

    return registers_.at(reg);
}

arch::Word& Storage::Mem(arch::Address address) {
    if (address >= arch::kMemorySize) {
        // TODO: throw
    }

    if (curr_config_.CodeSegmentIsBlocked() && address < curr_code_end_) {
        // TODO: throw
    }

    if (curr_config_.ConstantsSegmentIsBlocked() &&  //
        address >= curr_code_end_ &&                 //
        address < curr_constants_end_) {
        // TODO: throw
    }

    return memory_.at(address);
}

arch::Word& Storage::Flags() {
    return flags_;
}

}  // namespace karma::executor::detail
