#pragma once

#include <csetjmp>          // for jmp_buf
#include <cstddef>          // for size_t
#include <source_location>  // for source_location

#include "exceptions/exception.hpp"

namespace except::detail {

struct Node {
    explicit Node(size_t depth);

    int* Buff();
    void Raise(Type type, std::source_location);

    std::jmp_buf buf{};
    Exception exception;

    bool raised = false;
    size_t depth;
};

}  // namespace except::detail
