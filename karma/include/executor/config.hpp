#pragma once

#include <cstddef>        // for size_t
#include <cstdint>        // for uint32_t
#include <optional>       // for optional, nullopt
#include <unordered_set>  // for unordered_set

#include "executor.hpp"

namespace karma {

class Executor::Config {
   private:
    // NOLINTNEXTLINE(fuchsia-overloaded-operator)
    friend std::ostream& operator<<(std::ostream&, const Config&);

   private:
    using Registers = std::unordered_set<uint32_t>;

   private:
    class AccessConfig {
       private:
        friend class Config;
        // NOLINTNEXTLINE(fuchsia-overloaded-operator)
        friend std::ostream& operator<<(std::ostream&, const Config&);

       public:
        void SetBlockedRegisters(const Registers&);
        void SetBlockedRegisters(Registers&&);

        void BlockRegisters(const Registers&);
        void UnblockRegisters(const Registers&);

        void BlockUtilityRegisters();
        void UnblockUtilityRegisters();

        void SetCodeSegmentBlock(bool);
        void BlockCodeSegment();
        void UnblockCodeSegment();

        void SetConstantsSegmentBlock(bool);
        void BlockConstantsSegment();
        void UnblockConstantsSegment();

       private:
        // NOLINTNEXTLINE(fuchsia-overloaded-operator)
        AccessConfig& operator&=(const AccessConfig&);

        [[nodiscard]] bool RegisterIsBlocked(uint32_t reg) const;
        [[nodiscard]] bool CodeSegmentIsBlocked() const;
        [[nodiscard]] bool ConstantsSegmentIsBlocked() const;

       private:
        static const Registers kUtilityRegisters;

        Registers blocked_registers_;

        bool code_segment_blocked_{false};
        bool constants_segment_blocked_{false};
    };

   public:
    AccessConfig& WriteAccess();
    AccessConfig& ReadWriteAccess();

    void BoundStack(size_t stack_size);
    void UnboundStack();

    static Config Strict();
    static Config ExtraStrict();

    // NOLINTNEXTLINE(fuchsia-overloaded-operator)
    Config& operator&=(const Config&);
    // NOLINTNEXTLINE(fuchsia-overloaded-operator)
    Config operator&(const Config&);

    [[nodiscard]] bool RegisterIsWriteBlocked(uint32_t reg) const;
    [[nodiscard]] bool RegisterIsReadWriteBlocked(uint32_t reg) const;

    [[nodiscard]] bool CodeSegmentIsWriteBlocked() const;
    [[nodiscard]] bool CodeSegmentIsReadWriteBlocked() const;

    [[nodiscard]] bool ConstantsSegmentIsWriteBlocked() const;
    [[nodiscard]] bool ConstantsSegmentIsReadWriteBlocked() const;

    [[nodiscard]] size_t MaxStackSize() const;
    [[nodiscard]] size_t MinStackAddress() const;

   private:
    AccessConfig write_;
    AccessConfig read_write_;

    std::optional<size_t> max_stack_size_;
};

// NOLINTNEXTLINE(fuchsia-overloaded-operator)
std::ostream& operator<<(std::ostream&, const Executor::Config&);

}  // namespace karma
