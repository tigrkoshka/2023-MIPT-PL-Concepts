#pragma once

#include <cstdint>  // for uint32_t
#include <memory>   // for unique_ptr
#include <string>   // for string

#include "config.hpp"

namespace karma {

namespace executor::detail {

class Impl;

}  // namespace executor::detail

class Executor {
   public:
    using Config = executor::Config;

   public:
    explicit Executor(Config = Config());
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
    uint32_t MustExecute(const std::string& exec, const Config& = Config());
    uint32_t Execute(const std::string& exec, const Config& = Config());

   private:
    std::unique_ptr<executor::detail::Impl> impl_;
};

}  // namespace karma