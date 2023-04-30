#pragma once

#include <csetjmp>  // for jump_buf
#include <cstddef>  // for size_t
#include <string>   // for string
#include <stack> // for stack

#include "exception.hpp"

namespace except::detail {

struct Node {
    Node* prev{nullptr};
    std::jmp_buf buf{};
    Exception exception;
};

extern thread_local std::stack<Node*> stack;

}  // namespace except::detail
