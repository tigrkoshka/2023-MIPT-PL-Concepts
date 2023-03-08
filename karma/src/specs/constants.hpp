#pragma once

#include <string>         // for string
#include <unordered_map>  // for unordered_map

#include "architecture.hpp"

namespace karma::detail::specs::consts {

static const char kCharQuote = '\'';
static const char kStringQuote = '\"';

enum Type : arch::Word {
    UINT32,
    UINT64,
    DOUBLE,
    CHAR,
    STRING,
};

extern const std::unordered_map<Type, std::string> kTypeToName;
extern const std::unordered_map<std::string, Type> kNameToType;

}  // namespace karma::detail::specs::consts