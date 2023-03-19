#pragma once

#include <cstdint>   // for uint32_t
#include <iostream>  // for ostream, clog
#include <memory>    // for unique_ptr
#include <optional>  // for optional
#include <string>    // for string

#include "utils/error.hpp"

namespace karma {

namespace errors::executor {

struct Error;
struct InternalError;
struct ExecutionError;

}  // namespace errors::executor

class Executor {
   private:
    friend struct errors::executor::Error;
    friend struct errors::executor::InternalError;
    friend struct errors::executor::ExecutionError;

   public:
    class Config;

   private:
    class Storage;
    class ExecutorBase;
    class CommonExecutor;
    class RMExecutor;
    class RRExecutor;
    class RIExecutor;
    class JExecutor;
    class Impl;

   private:
    using ReturnCode      = uint32_t;
    using MaybeReturnCode = std::optional<ReturnCode>;

   public:
    Executor();
    explicit Executor(Config);
    ~Executor();

    // utils::traits::NonCopyableMovable

    // do not include utils/traits, because we don't want to expose
    // internal features of the karma library to the user

    // Non-copyable
    Executor(const Executor&)            = delete;
    Executor& operator=(const Executor&) = delete;

    // Movable
    Executor(Executor&&)            = default;
    Executor& operator=(Executor&&) = default;

   public:
    // cannot use default value for Config, because it is declared
    // in another file, which includes this one

    ReturnCode MustExecute(const std::string& exec_path,
                           std::ostream& log = std::clog);
    ReturnCode Execute(const std::string& exec_path,
                       std::ostream& log = std::clog);

    ReturnCode MustExecute(const std::string& exec_path,
                           const Config&,
                           std::ostream& log = std::clog);
    ReturnCode Execute(const std::string& exec_path,
                       const Config&,
                       std::ostream& log = std::clog);

   private:
    std::unique_ptr<Impl> impl_;
};

namespace errors::executor {

struct Error : errors::Error {
   private:
    friend class Executor::Impl;

   protected:
    explicit Error(const std::string& message)
        : errors::Error(message) {}
};

struct InternalError : Error {
   private:
    friend class Executor::Impl;

   private:
    struct Builder;

   private:
    explicit InternalError(const std::string& message)
        : Error("internal executor error: " + message) {}
};

struct ExecutionError : Error {
   private:
    friend class Executor::Storage;
    friend class Executor::CommonExecutor;
    friend class Executor::RIExecutor;
    friend class Executor::RRExecutor;
    friend class Executor::Impl;

   private:
    struct Builder;

   private:
    explicit ExecutionError(const std::string& message)
        : Error("execution error: " + message) {}
};

}  // namespace errors::executor

}  // namespace karma
