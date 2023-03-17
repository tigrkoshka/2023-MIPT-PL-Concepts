#pragma once

#include <memory>   // for shared_ptr
#include <utility>  // for move

#include "executor/executor.hpp"
#include "specs/architecture.hpp"
#include "utils/traits.hpp"

namespace karma {

class Executor::CommandExecutor : detail::utils::traits::NonCopyableMovable {
   public:
    // delete the default constructor defined in NonCopyableMovable
    CommandExecutor() = delete;

    // do not make the following constructor explicit to be able to use it
    // to create children of CommandExecutor in a concise way

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    CommandExecutor(std::shared_ptr<Storage> storage)
        : storage_(std::move(storage)){};

   protected:
    static const bool kInternalUse = true;

    void CheckPushAllowed();

    [[nodiscard]] detail::specs::arch::Word RReg(
        detail::specs::arch::Register, bool internal_usage = false) const;
    [[nodiscard]] detail::specs::arch::Word RMem(
        detail::specs::arch::Address) const;

    detail::specs::arch::Word& WReg(detail::specs::arch::Register,
                                    bool internal_usage = false);
    detail::specs::arch::Word& WMem(detail::specs::arch::Address);
    detail::specs::arch::Word& Flags();

   private:
    std::shared_ptr<Storage> storage_;
};

}  // namespace karma