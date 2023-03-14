#pragma once

#include <array>    // for array
#include <cstddef>  // for size_t
#include <utility>  // for pair
#include <vector>   // for vector

#include "config.hpp"
#include "exec/exec.hpp"
#include "specs/architecture.hpp"
#include "utils/traits.hpp"

namespace karma::executor::detail {

class Storage : karma::detail::utils::traits::NonCopyableMovable {
   private:
    using Word = karma::detail::specs::arch::Word;

   public:
    explicit Storage(Config config = Config())
        : base_config_(std::move(config)),
          memory_(karma::detail::specs::arch::kMemorySize) {}

    void PrepareForExecution(const karma::detail::exec::Data& exec_data,
                             const Config& config = Config());

    void CheckPushAllowed();

    // do not provide the const variants of the methods below,
    // because the storage is never meant to be const

    Word& Reg(karma::detail::specs::arch::Register);
    Word& Mem(karma::detail::specs::arch::Address);
    Word& Flags();

   private:
    // allocate the memory on the heap, and all the registers on the stack
    // to provide emulation that register operations are faster

    Config base_config_;
    Config curr_config_{base_config_};

    std::vector<Word> memory_;
    size_t curr_code_end_{0};
    size_t curr_constants_end_{0};

    std::array<Word, karma::detail::specs::arch::kNRegisters> registers_{};
    Word flags_{0};
};

}  // namespace karma::executor::detail