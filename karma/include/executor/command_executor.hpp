#pragma once

#include <cstdint>   // for uint32_t
#include <memory>    // for shared_ptr
#include <optional>  // for optional
#include <utility>

#include "specs/architecture.hpp"
#include "storage.hpp"
#include "utils/traits.hpp"

namespace karma::executor::detail {

class CommandExecutor : karma::detail::utils::traits::NonCopyableMovable {
   public:
    // delete the default constructor defined in NonCopyableMovable
    CommandExecutor() = delete;

    CommandExecutor(std::shared_ptr<Storage> storage)
        : storage_(std::move(storage)){};

   protected:
    void CheckPushAllowed();

    karma::detail::specs::arch::Word& Reg(karma::detail::specs::arch::Register);
    karma::detail::specs::arch::Word& Mem(karma::detail::specs::arch::Address);
    karma::detail::specs::arch::Word& Flags();

   private:
    std::shared_ptr<Storage> storage_;
};

}  // namespace karma::executor::detail