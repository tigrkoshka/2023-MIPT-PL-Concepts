#pragma once

#include <source_location>  // for source_location
#include <string>           // for string

namespace except {

enum class Type { A, B, C, D };
std::string Message(Type type);

struct Exception {
    Type type{};
    std::source_location source_location;
};

}  // namespace except
