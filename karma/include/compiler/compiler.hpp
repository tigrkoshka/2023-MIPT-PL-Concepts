#pragma once

#include <string>  // for string

#include "utils/error.hpp"

namespace karma {

namespace errors::compiler {

struct Error;
struct InternalError;
struct CompileError;

}  // namespace errors::compiler

class Compiler {
   private:
    friend struct errors::compiler::InternalError;
    friend struct errors::compiler::CompileError;

   private:
    class Entrypoint;
    class Labels;
    class ExecData;
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
                            const std::string& dst = "");
    static void Compile(const std::string& src, const std::string& dst = "");
};

namespace errors::compiler {

struct Error : errors::Error {
   protected:
    explicit Error(const std::string& message)
        : errors::Error(message) {}
};

struct InternalError : Error {
   private:
    friend class Compiler::File;
    friend class Compiler::FileCompiler;
    friend class Compiler::IncludesManager;
    friend class Compiler::Impl;

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
    friend class Compiler::Labels;
    friend class Compiler::File;
    friend class Compiler::IncludesManager;
    friend class Compiler::FileCompiler;
    friend class Compiler::Impl;

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