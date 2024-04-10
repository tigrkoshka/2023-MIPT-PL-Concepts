#include "config.hpp"

#include <algorithm>      // for min
#include <iostream>       // for ostream, ios
#include <optional>       // for optional
#include <unordered_set>  // for erase_if
#include <utility>        // for move

#include "executor/executor.hpp"
#include "specs/architecture.hpp"

namespace karma {

using Config = Executor::Config;

namespace arch = detail::specs::arch;

void Config::AccessConfig::SetBlockedRegisters(const Registers& regs) {
    blocked_registers_ = regs;
}

void Config::AccessConfig::SetBlockedRegisters(Registers&& regs) {
    blocked_registers_ = std::move(regs);
}

void Config::AccessConfig::BlockRegisters(const Registers& regs) {
    blocked_registers_.insert(regs.begin(), regs.end());
}

void Config::AccessConfig::UnblockRegisters(const Registers& regs) {
    auto remove = [&regs](auto const& elem) {
        return regs.contains(elem);
    };

    std::erase_if(blocked_registers_, remove);
}

void Config::AccessConfig::BlockUtilityRegisters() {
    BlockRegisters(kUtilityRegisters);
}

void Config::AccessConfig::UnblockUtilityRegisters() {
    UnblockRegisters(kUtilityRegisters);
}

void Config::AccessConfig::SetCodeSegmentBlock(bool block) {
    code_segment_blocked_ = block;
}

void Config::AccessConfig::BlockCodeSegment() {
    code_segment_blocked_ = true;
}

void Config::AccessConfig::UnblockCodeSegment() {
    code_segment_blocked_ = false;
}

void Config::AccessConfig::SetConstantsSegmentBlock(bool block) {
    constants_segment_blocked_ = block;
}

void Config::AccessConfig::BlockConstantsSegment() {
    constants_segment_blocked_ = true;
}

void Config::AccessConfig::UnblockConstantsSegment() {
    constants_segment_blocked_ = false;
}

// NOLINTNEXTLINE(fuchsia-overloaded-operator)
Config::AccessConfig& Config::AccessConfig::operator&=(
    const AccessConfig& rhs) {
    BlockRegisters(rhs.blocked_registers_);

    SetCodeSegmentBlock(code_segment_blocked_ || rhs.code_segment_blocked_);

    SetConstantsSegmentBlock(constants_segment_blocked_ ||
                             rhs.constants_segment_blocked_);

    return *this;
}

bool Config::AccessConfig::RegisterIsBlocked(arch::Register reg) const {
    return blocked_registers_.contains(reg);
}

bool Config::AccessConfig::CodeSegmentIsBlocked() const {
    return code_segment_blocked_;
}

bool Config::AccessConfig::ConstantsSegmentIsBlocked() const {
    return constants_segment_blocked_;
}

const Config::Registers Config::AccessConfig::kUtilityRegisters = {
    arch::kCallFrameRegister,
    arch::kStackRegister,
    arch::kInstructionRegister,
};

Config::AccessConfig& Config::WriteAccess() {
    return write_;
}

Config::AccessConfig& Config::ReadWriteAccess() {
    return read_write_;
}

void Config::BoundStack(size_t stack_size) {
    if (stack_size >= arch::kMemorySize) {
        max_stack_size_ = std::nullopt;
        return;
    }

    max_stack_size_ = stack_size;
}

void Config::UnboundStack() {
    max_stack_size_ = std::nullopt;
}

Config Config::Strict() {
    Config config;

    config.read_write_.BlockRegisters({
        arch::kCallFrameRegister,
        arch::kInstructionRegister,
    });
    config.write_.BlockRegisters({arch::kStackRegister});

    config.read_write_.BlockCodeSegment();
    config.write_.BlockConstantsSegment();

    return config;
}

Config Config::ExtraStrict() {
    Config config;

    config.read_write_.BlockUtilityRegisters();
    config.read_write_.BlockCodeSegment();
    config.read_write_.BlockConstantsSegment();

    return config;
}

// NOLINTNEXTLINE(fuchsia-overloaded-operator)
Config& Config::operator&=(const Config& rhs) {
    write_ &= rhs.write_;
    read_write_ &= rhs.read_write_;

    if (max_stack_size_ && rhs.max_stack_size_) {
        BoundStack(std::min(*max_stack_size_, *rhs.max_stack_size_));
    } else if (rhs.max_stack_size_) {
        BoundStack(*rhs.max_stack_size_);
    }

    return *this;
}

// NOLINTNEXTLINE(fuchsia-overloaded-operator)
Config Config::operator&(const Config& rhs) {
    Config lhs = *this;
    lhs &= rhs;
    return lhs;
}

bool Config::RegisterIsWriteBlocked(uint32_t reg) const {
    return write_.RegisterIsBlocked(reg) ||  //
           read_write_.RegisterIsBlocked(reg);
}

bool Config::RegisterIsReadWriteBlocked(uint32_t reg) const {
    return read_write_.RegisterIsBlocked(reg);
}

bool Config::CodeSegmentIsWriteBlocked() const {
    return write_.CodeSegmentIsBlocked() ||  //
           read_write_.CodeSegmentIsBlocked();
}

bool Config::CodeSegmentIsReadWriteBlocked() const {
    return read_write_.CodeSegmentIsBlocked();
}

bool Config::ConstantsSegmentIsWriteBlocked() const {
    return write_.ConstantsSegmentIsBlocked() ||  //
           read_write_.ConstantsSegmentIsBlocked();
}

bool Config::ConstantsSegmentIsReadWriteBlocked() const {
    return read_write_.ConstantsSegmentIsBlocked();
}

size_t Config::MaxStackSize() const {
    if (!max_stack_size_) {
        return arch::kMemorySize;
    }

    return *max_stack_size_;
}

size_t Config::MinStackAddress() const {
    return arch::kMemorySize - MaxStackSize();
}

// NOLINTNEXTLINE(fuchsia-overloaded-operator)
std::ostream& operator<<(std::ostream& out, const Config& config) {
    auto print_registers = [&out](const Config::Registers& registers) {
        out << "{";

        bool comma = false;
        for (const arch::Register reg : registers) {
            if (!arch::kRegisterNumToName.contains(reg)) {
                continue;
            }

            if (std::exchange(comma, true)) {
                out << ", ";
            }

            out << arch::kRegisterNumToName.at(reg);
        }

        out << "}";
    };

    out << "registers blocks:\n";

    out << "    write:      ";
    print_registers(config.write_.blocked_registers_);
    out << '\n';

    out << "    read-write: ";
    print_registers(config.read_write_.blocked_registers_);
    out << '\n';

    // remember formatting flags before setting boolalpha
    const std::ios_base::fmtflags old = out.setf(std::ios_base::boolalpha);

    out << "code segment blocks:" << '\n'
        << "    write: " << config.CodeSegmentIsWriteBlocked() << '\n'
        << "    read:  " << config.CodeSegmentIsReadWriteBlocked() << '\n'
        << "constants segment blocks:" << '\n'
        << "    write: " << config.ConstantsSegmentIsWriteBlocked() << '\n'
        << "    read:  " << config.ConstantsSegmentIsReadWriteBlocked() << '\n';

    // restore flags
    out.flags(old);

    out << "stack: ";
    if (!config.max_stack_size_) {
        out << "unbounded";
    } else {
        out << "\n    max size: " << *config.max_stack_size_;
    }

    return out;
}

}  // namespace karma
