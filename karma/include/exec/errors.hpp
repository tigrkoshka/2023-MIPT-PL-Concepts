#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

#include "specs/architecture.hpp"
#include "utils/error.hpp"

namespace karma::errors::exec {

struct Error : errors::Error {
   protected:
    explicit Error(const std::string& message)
        : errors::Error(message) {}
};

struct ExecFileError : Error {
   private:
    ExecFileError(const std::string& message, const std::string& path)
        : Error(path + ": exec file error: " + message) {}

   public:
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