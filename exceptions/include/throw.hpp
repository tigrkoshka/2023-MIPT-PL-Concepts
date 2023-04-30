#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

#include "exception.hpp"

namespace except::detail {

enum class Status { NO_EXCEPTION, RAISED, HANDLED };

void Throw(Type type, std::string file, size_t line);

}  // namespace except::detail
