#include "config.hpp"

#include <algorithm>      // for min
#include <optional>       // for optional
#include <unordered_set>  // for erase_if
#include <utility>        // for move

#include "executor/executor.hpp"
#include "specs/architecture.hpp"

namespace karma {

namespace arch = detail::specs::arch;

Executor::Config& Executor::Config::SetBlockedRegisters(const Registers& regs) {
    blocked_registers_ = regs;
    return *this;
}

Executor::Config& Executor::Config::SetBlockedRegisters(Registers&& regs) {
    blocked_registers_ = std::move(regs);
    return *this;
}

Executor::Config& Executor::Config::BlockRegisters(const Registers& regs) {
    blocked_registers_.insert(regs.begin(), regs.end());
    return *this;
}

Executor::Config& Executor::Config::UnblockRegisters(const Registers& regs) {
    auto remove = [&regs](auto const& elem) {
        return regs.contains(elem);
    };

    std::erase_if(blocked_registers_, remove);
    return *this;
}

Executor::Config& Executor::Config::BlockUtilityRegisters() {
    BlockRegisters(kUtilityRegisters);
    return *this;
}

Executor::Config& Executor::Config::UnblockUtilityRegisters() {
    UnblockRegisters(kUtilityRegisters);
    return *this;
}

Executor::Config& Executor::Config::SetCodeSegmentBlock(bool block) {
    code_segment_blocked_ = block;
    return *this;
}

Executor::Config& Executor::Config::BlockCodeSegment() {
    code_segment_blocked_ = true;
    return *this;
}

Executor::Config& Executor::Config::UnblockCodeSegment() {
    code_segment_blocked_ = false;
    return *this;
}

Executor::Config& Executor::Config::SetConstantsSegmentBlock(bool block) {
    constants_segment_blocked_ = block;
    return *this;
}

Executor::Config& Executor::Config::BlockConstantsSegment() {
    constants_segment_blocked_ = true;
    return *this;
}

Executor::Config& Executor::Config::UnblockConstantsSegment() {
    constants_segment_blocked_ = false;
    return *this;
}

Executor::Config& Executor::Config::BoundStack(size_t stack_size) {
    if (stack_size >= arch::kMemorySize) {
        max_stack_size_ = std::nullopt;
        return *this;
    }

    max_stack_size_ = stack_size;
    return *this;
}

Executor::Config& Executor::Config::UnboundStack() {
    max_stack_size_ = std::nullopt;
    return *this;
}

Executor::Config& Executor::Config::Strict() {
    BlockUtilityRegisters();
    BlockCodeSegment();
    BlockConstantsSegment();

    return *this;
}

// NOLINTNEXTLINE(fuchsia-overloaded-operator)
Executor::Config& Executor::Config::operator&=(const Executor::Config& other) {
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

bool Executor::Config::RegisterIsBlocked(arch::Register reg) const {
    return blocked_registers_.contains(reg);
}

bool Executor::Config::CodeSegmentIsBlocked() const {
    return code_segment_blocked_;
}

bool Executor::Config::ConstantsSegmentIsBlocked() const {
    return constants_segment_blocked_;
}

size_t Executor::Config::MaxStackSize() const {
    if (!max_stack_size_) {
        return arch::kMemorySize;
    }

    return *max_stack_size_;
}

size_t Executor::Config::MinStackAddress() const {
    return arch::kMemorySize - MaxStackSize();
}

const Executor::Config::Registers Executor::Config::kUtilityRegisters = {
    arch::kCallFrameRegister,
    arch::kStackRegister,
    arch::kInstructionRegister,
};

}  // namespace karma