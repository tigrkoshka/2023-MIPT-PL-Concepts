#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

#include "utils/error.hpp"
#include "utils/logger.hpp"

namespace karma {

namespace errors::compiler {

struct Error;
struct InternalError;
struct CompileError;

}  // namespace errors::compiler

class Compiler {
   private:
    friend struct errors::compiler::Error;
    friend struct errors::compiler::InternalError;
    friend struct errors::compiler::CompileError;

   private:
    class Entrypoint;
    class Labels;
    class Data;
    class File;
    class IncludesManager;
    class FileCompiler;
    class Impl;

   public:
    // utils::traits::Static

    // do not include utils/traits, because we don't want to expose
    // internal features of the karma library to the user

    Compiler()  = delete;
    ~Compiler() = delete;

    Compiler(const Compiler&)            = delete;
    Compiler& operator=(const Compiler&) = delete;

    Compiler(Compiler&&)            = delete;
    Compiler& operator=(Compiler&&) = delete;

   public:
    static void MustCompile(const std::string& src,
                            const std::string& dst = "",
                            size_t n_workers       = kDefaultWorkers,
                            Logger log             = Logger::NoOp());
    static void Compile(const std::string& src,
                        const std::string& dst = "",
                        size_t n_workers       = kDefaultWorkers,
                        Logger log             = Logger::NoOp());

    static void MustCompile(const std::string& src,
                            const std::string& dst,
                            Logger log);
    static void Compile(const std::string& src,
                        const std::string& dst,
                        Logger log);

    static void MustCompile(const std::string& src,
                            size_t n_workers,
                            Logger log = Logger::NoOp());
    static void Compile(const std::string& src,
                        size_t n_workers,
                        Logger log = Logger::NoOp());

    static void MustCompile(const std::string& src, Logger log);
    static void Compile(const std::string& src, Logger log);

   private:
    static const size_t kDefaultWorkers;
};

namespace errors::compiler {

struct Error : errors::Error {
   private:
    friend class Compiler::Impl;

   protected:
    explicit Error(const std::string& message)
        : errors::Error(message) {}
};

struct InternalError : Error {
   private:
    friend class Compiler::Entrypoint;
    friend class Compiler::Data;
    friend class Compiler::File;
    friend class Compiler::FileCompiler;

   private:
    struct Builder;

   private:
    explicit InternalError(const std::string& message)
        : Error("internal compiler error: " + message) {}

    InternalError(const std::string& message, const std::string& where)
        : Error("internal compiler error " + where + ": " + message) {}
};

struct CompileError : Error {
   private:
    friend class Compiler::Entrypoint;
    friend class Compiler::Labels;
    friend class Compiler::Data;
    friend class Compiler::File;
    friend class Compiler::IncludesManager;
    friend class Compiler::FileCompiler;

   private:
    struct Builder;

   private:
    explicit CompileError(const std::string& message)
        : Error("compile error: " + message) {}

    CompileError(const std::string& message, const std::string& where)
        : Error("compile error " + where + ": " + message) {}
};

}  // namespace errors::compiler

}  // namespace karma
