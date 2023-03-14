#pragma once

#include <cstdint>        // for int32_t
#include <optional>       // for optional, nullopt
#include <unordered_set>  // for unordered_set

#include "specs/architecture.hpp"

namespace karma::executor {

class Config {
   private:
    using Registers = std::unordered_set<karma::detail::specs::arch::Register>;

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

   private:
    static const Registers kUtilityRegisters;

    Registers blocked_registers_;
    bool block_code_segment_{false};
    bool block_constants_segment_{false};
    std::optional<size_t> max_stack_size_;
};

// NOLINTNEXTLINE(fuchsia-overloaded-operator)
inline Config operator&(Config lhs, const Config& rhs) {
    lhs &= rhs;
    return lhs;
}

}  // namespace karma::executor