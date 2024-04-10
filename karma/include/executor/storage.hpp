#pragma once

#include <array>    // for array
#include <cstddef>  // for size_t
#include <ostream>  // for ostream
#include <utility>  // for pair
#include <vector>   // for vector

#include "exec/exec.hpp"
#include "executor/config.hpp"
#include "executor/errors.hpp"
#include "executor/executor.hpp"
#include "specs/architecture.hpp"
#include "utils/traits.hpp"

namespace karma {

class Executor::Storage : detail::utils::traits::NonCopyableMovable {
   private:
    using ExecutionError = errors::executor::ExecutionError::Builder;
    using Word           = detail::specs::arch::Word;

   public:
    explicit Storage(Config config)
        : base_config_(std::move(config)) {}

    void PrepareForExecution(const Exec::Data& exec_data,
                             const Config& config,
                             std::ostream& log);

    void CheckPushAllowed() const;

    [[nodiscard]] Word RReg(detail::specs::arch::Register,
                            bool internal_usage = false) const;
    [[nodiscard]] Word RMem(detail::specs::arch::Address,
                            bool internal_usage = false) const;

    Word& WReg(detail::specs::arch::Register, bool internal_usage = false);
    Word& WMem(detail::specs::arch::Address, bool internal_usage = false);
    Word& Flags();

   private:
    Config base_config_;
    Config curr_config_{base_config_};

    // allocate the memory on the heap, and all the registers on the stack
    // to provide emulation that register operations are faster

    std::vector<Word> memory_ =
        std::vector<Word>(detail::specs::arch::kMemorySize);
    size_t curr_code_end_{0};
    size_t curr_constants_end_{0};

    std::array<Word, detail::specs::arch::kNRegisters> registers_{};
    Word flags_{0};
};

}  // namespace karma
