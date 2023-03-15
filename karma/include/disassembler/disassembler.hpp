#pragma once

#include <ostream>  // for ostream
#include <string>   // for string

#include "utils/error.hpp"

namespace karma {

namespace errors::disassembler {

struct Error;
struct InternalError;
struct DisassembleError;

}  // namespace errors::disassembler

class Disassembler {
   private:
    friend struct errors::disassembler::InternalError;
    friend struct errors::disassembler::DisassembleError;

   private:
    class Impl;

   public:
    // utils::traits::Static

    // do not include utils/traits, because we don't want to expose
    // internal features of the karma library to the user

    Disassembler()  = delete;
    ~Disassembler() = delete;

    Disassembler(const Disassembler&)            = delete;
    Disassembler& operator=(const Disassembler&) = delete;

    Disassembler(Disassembler&&)            = delete;
    Disassembler& operator=(Disassembler&&) = delete;

   public:
    static void MustDisassemble(const std::string& exec_path,
                                std::ostream& out);

    static void Disassemble(const std::string& exec_path, std::ostream& out);

    static void MustDisassemble(const std::string& exec_path,
                                const std::string& dst = "");

    static void Disassemble(const std::string& exec_path,
                            const std::string& dst = "");
};

namespace errors::disassembler {

struct Error : errors::Error {
   protected:
    explicit Error(const std::string& message)
        : errors::Error(message) {}
};

struct InternalError : Error {
   private:
    friend class Disassembler::Impl;

   private:
    struct Builder;

   private:
    explicit InternalError(const std::string& message)
        : Error("internal disassembler error: " + message) {}
};

struct DisassembleError : Error {
   private:
    friend class Disassembler::Impl;

   private:
    struct Builder;

   private:
    explicit DisassembleError(const std::string& message)
        : Error("disassembling error: " + message) {}
};

}  // namespace errors::disassembler

}  // namespace karma
