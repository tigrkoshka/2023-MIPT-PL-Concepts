#include "config.hpp"

#include <algorithm>      // for min
#include <optional>       // for optional
#include <unordered_set>  // for erase_if
#include <utility>        // for move

#include "specs/architecture.hpp"

namespace karma::executor {

namespace arch = karma::detail::specs::arch;

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
    code_segment_blocked_ = block;
    return *this;
}

Config& Config::BlockCodeSegment() {
    code_segment_blocked_ = true;
    return *this;
}

Config& Config::UnblockCodeSegment() {
    code_segment_blocked_ = false;
    return *this;
}

Config& Config::SetConstantsSegmentBlock(bool block) {
    constants_segment_blocked_ = block;
    return *this;
}

Config& Config::BlockConstantsSegment() {
    constants_segment_blocked_ = true;
    return *this;
}

Config& Config::UnblockConstantsSegment() {
    constants_segment_blocked_ = false;
    return *this;
}

Config& Config::BoundStack(size_t stack_size) {
    if (stack_size >= arch::kMemorySize) {
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

    SetCodeSegmentBlock(code_segment_blocked_ || other.code_segment_blocked_);

    SetConstantsSegmentBlock(constants_segment_blocked_ ||
                             other.constants_segment_blocked_);

    if (max_stack_size_ && other.max_stack_size_) {
        BoundStack(std::min(*max_stack_size_, *other.max_stack_size_));
    } else if (other.max_stack_size_) {
        BoundStack(*other.max_stack_size_);
    }

    return *this;
}

bool Config::RegisterIsBlocked(arch::Register reg) {
    return blocked_registers_.contains(reg);
}

bool Config::CodeSegmentIsBlocked() {
    return code_segment_blocked_;
}

bool Config::ConstantsSegmentIsBlocked() {
    return constants_segment_blocked_;
}

size_t Config::MaxStackSize() {
    if (!max_stack_size_) {
        return arch::kMemorySize;
    }

    return *max_stack_size_;
}

size_t Config::MinStackAddress() {
    return arch::kMemorySize - MaxStackSize();
}

const Config::Registers Config::kUtilityRegisters = {
    karma::detail::specs::arch::kCallFrameRegister,
    karma::detail::specs::arch::kStackRegister,
    karma::detail::specs::arch::kInstructionRegister,
};

}  // namespace karma::executor