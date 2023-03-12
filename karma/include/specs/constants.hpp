#pragma once

#include <string>         // for string
#include <unordered_map>  // for unordered_map

#include "architecture.hpp"

namespace karma::detail::specs::consts {

constexpr char kCharQuote   = '\'';
constexpr char kStringQuote = '\"';

enum Type : arch::Word {
    UINT32,
    UINT64,
    DOUBLE,
    CHAR,
    STRING,
};

using UInt32 = arch::Word;
using UInt64 = arch::TwoWords;
using Double = arch::Double;
using Char   = char;
using String = std::string;

// 15 was chosen, because empirically double errors when dealing with constants
// start when precision is 17 (leave one extra precision point as a guarantee)
constexpr int32_t kDoublePrecision = 15;
constexpr Char kStringEnd          = '\0';

extern const std::unordered_map<Type, std::string> kTypeToName;
extern const std::unordered_map<std::string, Type> kNameToType;

}  // namespace karma::detail::specs::consts