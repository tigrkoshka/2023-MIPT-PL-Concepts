#pragma once

#include <cstddef>        // for size_t
#include <cstdint>        // for uint32_t
#include <optional>       // for optional, nullopt
#include <unordered_set>  // for unordered_set

#include "executor.hpp"

namespace karma {

class Executor::Config {
   private:
    using Registers = std::unordered_set<uint32_t>;

   public:
    Config& SetBlockedRegisters(const Registers&);
    Config& SetBlockedRegisters(Registers&&);

    Config& BlockRegisters(const Registers&);
    Config& UnblockRegisters(const Registers&);

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
    // NOLINTNEXTLINE(fuchsia-overloaded-operator)
    Executor::Config operator&(const Config&);

    [[nodiscard]] bool RegisterIsBlocked(uint32_t reg) const;
    [[nodiscard]] bool CodeSegmentIsBlocked() const;
    [[nodiscard]] bool ConstantsSegmentIsBlocked() const;
    [[nodiscard]] size_t MaxStackSize() const;
    [[nodiscard]] size_t MinStackAddress() const;

   private:
    static const Registers kUtilityRegisters;

    Registers blocked_registers_;
    bool code_segment_blocked_{false};
    bool constants_segment_blocked_{false};
    std::optional<size_t> max_stack_size_;
};

}  // namespace karma