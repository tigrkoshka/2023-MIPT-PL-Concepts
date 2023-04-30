#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

namespace except {

enum class Type { A, B, C, D };
std::string Message(Type type);

struct Exception {
    Type type{};
    const char* file{nullptr};
    size_t line{0};
};

}  // namespace except
