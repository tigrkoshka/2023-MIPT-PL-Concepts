#pragma once

namespace except {

struct Exception {
    virtual ~Exception() = default;
};

struct LogicError : Exception {};
struct InvalidArgument : LogicError {};
struct DomainError : LogicError {};
struct LengthError : LogicError {};
struct OutOfRange : LogicError {};

struct RuntimeError : Exception {};
struct RangeError : RuntimeError {};
struct OverflowError : RuntimeError {};
struct UnderflowError : RuntimeError {};
struct SystemError : RuntimeError {};

}  // namespace except
