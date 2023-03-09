#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

#include "../specs/architecture.hpp"
#include "error.hpp"

namespace karma::errors::exec {

struct Error : errors::Error {
   protected:
    explicit Error(const std::string& message)
        : errors::Error(message) {}
};

struct ExecFileError : Error {
   private:
    explicit ExecFileError(const std::string& message)
        : Error("exec file error: " + message) {}

   public:
    static ExecFileError FailedToOpen();

    static ExecFileError TooSmallForHeader(size_t size);

    static ExecFileError InvalidExecSize(size_t exec_size,
                                         size_t code_size,
                                         size_t consts_size,
                                         size_t data_size);

    static ExecFileError NoTrailingZeroInIntro(const std::string& intro);

    static ExecFileError InvalidIntroString(const std::string& intro);

    static ExecFileError InvalidProcessorID(
        detail::specs::arch::Word processor_id);
};

}  // namespace karma::errors::exec