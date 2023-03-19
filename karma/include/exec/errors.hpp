#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

#include "exec/exec.hpp"
#include "specs/architecture.hpp"
#include "utils/traits.hpp"

namespace karma::errors::exec {

struct ExecFileError::Builder : detail::utils::traits::Static {
    static ExecFileError FailedToOpen(const std::string& path);

    static ExecFileError TooSmallForHeader(size_t size,
                                           const std::string& path);

    static ExecFileError TooBigForMemory(size_t size, const std::string& path);

    static ExecFileError InvalidExecSize(size_t exec_size,
                                         size_t code_size,
                                         size_t consts_size,
                                         const std::string& path);

    static ExecFileError NoTrailingZeroInIntro(const std::string& intro,
                                               const std::string& path);

    static ExecFileError InvalidIntroString(const std::string& intro,
                                            const std::string& path);

    static ExecFileError InvalidProcessorID(
        detail::specs::arch::Word processor_id, const std::string& path);
};

}  // namespace karma::errors::exec
