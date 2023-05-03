#pragma once

#include <csetjmp>          // for jmp_buf
#include <cstddef>          // for size_t
#include <source_location>  // for source_location

#include "data.hpp"

namespace except::detail {

struct Node {
    Node() = delete;
    explicit Node(size_t depth);

    ~Node() = default;

    // Non-copyable
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    // Movable
    Node(Node&&) = default;
    Node& operator=(Node&&) = default;

    int* Buff();
    void Raise(Data, std::source_location);

    std::jmp_buf buf{};
    bool raised = false;
    size_t depth;

    Data data;
    std::source_location source_location;
};

}  // namespace except::detail
