#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

namespace except {

enum class Type { A, B, C, D };
std::string Message(Type type);

struct Exception {
    Type type{};
    std::string file;
    size_t line{};
};

}  // namespace except
