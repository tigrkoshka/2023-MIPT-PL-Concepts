#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

#include "exception.hpp"

namespace except::detail {

void Throw(Type type, const char* file, size_t line);

}  // namespace except::detail
