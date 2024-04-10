#pragma once

#include <cstdint>        // for int32_t
#include <string>         // for string
#include <unordered_map>  // for unordered_map

#include "specs/architecture.hpp"

namespace karma::detail::specs::consts {

////////////////////////////////////////////////////////////////////////////////
///                                   Types                                  ///
////////////////////////////////////////////////////////////////////////////////

enum Type : arch::Word {
    UINT32,
    UINT64,
    DOUBLE,
    CHAR,
    STRING,
};

extern const std::unordered_map<Type, std::string> kTypeToName;
extern const std::unordered_map<std::string, Type> kNameToType;

using UInt32 = arch::Word;
using UInt64 = arch::TwoWords;
using Double = arch::Double;
using Char   = char;

////////////////////////////////////////////////////////////////////////////////
///                                Type specs                                ///
////////////////////////////////////////////////////////////////////////////////

// 15 was chosen, because empirically double errors when dealing with constants
// start when precision is 17 (leave one extra precision point as a guarantee)
constexpr int32_t kDoublePrecision = 15;
constexpr char kCharQuote          = '\'';
constexpr char kStringQuote        = '\"';
constexpr Char kStringEnd          = '\0';

}  // namespace karma::detail::specs::consts
