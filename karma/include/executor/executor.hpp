#pragma once

#include <cstdint>   // for uint32_t
#include <memory>    // for unique_ptr
#include <string>    // for string

namespace karma {

namespace executor::detail {

class Impl;

}  // namespace executor::detail

class Executor {
   public:
    Executor();
    ~Executor();

    // do not include utils/traits, because we don't want to expose
    // internal features of the karma library to the user

    // Non-copyable
    Executor(const Executor&)            = delete;
    Executor& operator=(const Executor&) = delete;

    // Movable
    Executor(Executor&&)            = default;
    Executor& operator=(Executor&&) = default;

   public:
    uint32_t MustExecute(const std::string& exec_path);
    uint32_t Execute(const std::string& exec_path);

   private:
    std::unique_ptr<executor::detail::Impl> impl_;
};

}  // namespace karma