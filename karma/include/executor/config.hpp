#pragma once

#include <cstddef>        // for size_t
#include <optional>       // for optional, nullopt
#include <unordered_set>  // for unordered_set

#include "executor.hpp"

namespace karma {

class Executor::Config {
   private:
    using Registers = std::unordered_set<uint32_t>;

   public:
    Config& SetBlockedRegisters(const Registers& registers);
    Config& SetBlockedRegisters(Registers&& registers);

    Config& BlockRegisters(const Registers& registers);
    Config& UnblockRegisters(const Registers& registers);

    Config& BlockUtilityRegisters();
    Config& UnblockUtilityRegisters();

    Config& SetCodeSegmentBlock(bool);
    Config& BlockCodeSegment();
    Config& UnblockCodeSegment();

    Config& SetConstantsSegmentBlock(bool);
    Config& BlockConstantsSegment();
    Config& UnblockConstantsSegment();

    Config& BoundStack(size_t stack_size);
    Config& UnboundStack();

    Config& Strict();

    // NOLINTNEXTLINE(fuchsia-overloaded-operator)
    Config& operator&=(const Config&);

    bool RegisterIsBlocked(uint32_t reg);
    bool CodeSegmentIsBlocked();
    bool ConstantsSegmentIsBlocked();
    size_t MaxStackSize();
    size_t MinStackAddress();

   private:
    static const Registers kUtilityRegisters;

    Registers blocked_registers_;
    bool code_segment_blocked_{false};
    bool constants_segment_blocked_{false};
    std::optional<size_t> max_stack_size_;
};

// NOLINTNEXTLINE(fuchsia-overloaded-operator)
inline Executor::Config operator&(Executor::Config lhs,
                                  const Executor::Config& rhs) {
    lhs &= rhs;
    return lhs;
}

}  // namespace karma