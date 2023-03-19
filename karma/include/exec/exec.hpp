#pragma once

#include <string>  // for string
#include <vector>  // for vector

#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "utils/error.hpp"
#include "utils/traits.hpp"

namespace karma {

namespace errors::exec {

struct Error;
struct ExecFileError;

}  // namespace errors::exec

class Exec : detail::utils::traits::Static {
   private:
    using ExecFileError = errors::exec::ExecFileError;

   public:
    struct Data : detail::utils::traits::NonCopyableMovable {
        detail::specs::arch::Address entrypoint{0};
        detail::specs::arch::Address initial_stack{
            detail::specs::arch::kMemorySize - 1};

        std::vector<detail::specs::cmd::Bin> code;
        std::vector<detail::specs::arch::Word> constants;
    };

    static void Write(const Data& data, const std::string& exec_path);
    static Data Read(const std::string& exec_path);
};

namespace errors::exec {

struct Error : errors::Error {
   protected:
    explicit Error(const std::string& message)
        : errors::Error(message) {}
};

struct ExecFileError : Error {
   private:
    friend class karma::Exec;

   private:
    struct Builder;

   private:
    ExecFileError(const std::string& message, const std::string& path)
        : Error(path + ": exec file error: " + message) {}
};

}  // namespace errors::exec

}  // namespace karma
