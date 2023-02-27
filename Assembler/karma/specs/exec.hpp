#pragma once

#include <cstddef>    // for size_t
#include <stdexcept>  // for runtime_error
#include <string>     // for string
#include <vector>     // for vector

#include "architecture.hpp"
#include "commands.hpp"

namespace karma::detail::specs::exec {

struct Data {
    specs::arch::Address entrypoint;
    specs::arch::Address initial_stack;

    std::vector<cmd::Bin> code;
    std::vector<arch::Word> constants;
    std::vector<arch::Word> data;
};

struct Error : std::runtime_error {
   protected:
    explicit Error(const std::string& message)
        : std::runtime_error(message) {}
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

    static ExecFileError InvalidProcessorID(arch::types::Word processor_id);
};

void Write(const Data& data, const std::string& exec_path);
Data Read(const std::string& exec_path);

}  // namespace karma::detail::specs::exec
