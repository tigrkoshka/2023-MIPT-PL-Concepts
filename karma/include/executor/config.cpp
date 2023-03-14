#include "config.hpp"

#include <algorithm>      // for min
#include <unordered_set>  // for erase_if

namespace karma::executor {

Config& Config::SetBlockedRegisters(const Registers& registers) {
    blocked_registers_ = registers;
    return *this;
}

Config& Config::SetBlockedRegisters(Registers&& registers) {
    blocked_registers_ = std::move(registers);
    return *this;
}

Config& Config::BlockRegisters(const Registers& registers) {
    blocked_registers_.insert(registers.begin(), registers.end());
    return *this;
}

Config& Config::UnblockRegisters(const Registers& registers) {
    auto remove = [&registers](auto const& elem) {
        return registers.contains(elem);
    };

    std::erase_if(blocked_registers_, remove);
    return *this;
}

Config& Config::BlockUtilityRegisters() {
    BlockRegisters(kUtilityRegisters);
    return *this;
}

Config& Config::UnblockUtilityRegisters() {
    UnblockRegisters(kUtilityRegisters);
    return *this;
}

Config& Config::SetCodeSegmentBlock(bool block) {
    block_code_segment_ = block;
    return *this;
}

Config& Config::BlockCodeSegment() {
    block_code_segment_ = true;
    return *this;
}

Config& Config::UnblockCodeSegment() {
    block_code_segment_ = false;
    return *this;
}

Config& Config::SetConstantsSegmentBlock(bool block) {
    block_constants_segment_ = block;
    return *this;
}

Config& Config::BlockConstantsSegment() {
    block_constants_segment_ = true;
    return *this;
}

Config& Config::UnblockConstantsSegment() {
    block_constants_segment_ = false;
    return *this;
}

Config& Config::BoundStack(size_t stack_size) {
    if (stack_size >= karma::detail::specs::arch::kMemorySize) {
        max_stack_size_ = std::nullopt;
    } else {
        max_stack_size_ = stack_size;
    }

    return *this;
}

Config& Config::UnboundStack() {
    max_stack_size_ = std::nullopt;
    return *this;
}

Config& Config::Strict() {
    BlockUtilityRegisters();
    BlockCodeSegment();
    BlockConstantsSegment();

    return *this;
}

Config& Config::operator&=(const Config& other) {
    BlockRegisters(other.blocked_registers_);

    SetCodeSegmentBlock(block_code_segment_ || other.block_code_segment_);

    SetConstantsSegmentBlock(block_constants_segment_ ||
                             other.block_constants_segment_);

    if (max_stack_size_ && other.max_stack_size_) {
        BoundStack(std::min(*max_stack_size_, *other.max_stack_size_));
    } else if (other.max_stack_size_) {
        BoundStack(*other.max_stack_size_);
    }

    return *this;
}

const Config::Registers Config::kUtilityRegisters = {
    karma::detail::specs::arch::kCallFrameRegister,
    karma::detail::specs::arch::kStackRegister,
    karma::detail::specs::arch::kInstructionRegister,
};

}  // namespace karma::executor