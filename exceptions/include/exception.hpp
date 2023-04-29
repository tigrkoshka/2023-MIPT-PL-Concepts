#pragma once

#include <csetjmp>  // for jmp_buf
#include <cstddef>  // for size_t
#include <string>   // for string

namespace except {

enum class Type { A, B, C, D };
std::string Message(Type type);

namespace details {

struct Node {
    Node* prev{nullptr};
    std::jmp_buf buf{};
    Type type{};
    std::string file{};
    size_t line{};
};

enum class Status { NO_EXCEPTION, RAISED, HANDLED };

}  // namespace details
}  // namespace except
