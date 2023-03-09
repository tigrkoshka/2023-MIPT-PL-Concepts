#pragma once

#include <memory>  // for shared_ptr
#include <string>  // for string

namespace karma {

namespace executor::detail {

class Impl;

}  // namespace executor::detail

class Executor {
   public:
    Executor();

    ~Executor() = default;

    // Non-copyable
    Executor(const Executor&)            = delete;
    Executor& operator=(const Executor&) = delete;

    // Non-movable
    Executor(Executor&&)            = delete;
    Executor& operator=(Executor&&) = delete;

   public:
    void MustExecute(const std::string& exec_path);
    void Execute(const std::string& exec_path);

   private:
    std::shared_ptr<executor::detail::Impl> impl_;
};

}  // namespace karma