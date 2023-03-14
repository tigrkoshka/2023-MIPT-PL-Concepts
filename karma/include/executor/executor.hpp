#pragma once

#include <cstdint>  // for uint32_t
#include <memory>   // for unique_ptr
#include <string>   // for string

namespace karma {

class Executor {
   public:
    class Config;

   public:
    Executor();
    explicit Executor(Config);
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
    // cannot use default value for Config, because it is declared
    // in another file, which includes this one

    uint32_t MustExecute(const std::string& exec);
    uint32_t Execute(const std::string& exec);

    uint32_t MustExecute(const std::string& exec, const Config&);
    uint32_t Execute(const std::string& exec, const Config&);

   private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace karma