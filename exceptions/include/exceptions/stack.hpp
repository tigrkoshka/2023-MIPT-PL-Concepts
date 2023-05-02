#pragma once

#include <csetjmp>          // for jump_buf
#include <optional>         // for optional
#include <source_location>  // for source_location
#include <stack>            // for stack

#include "exceptions/exception.hpp"

namespace except::detail {

std::pair<bool, int*> StartTry();
bool Catch();
bool TryCatch(Type);
[[noreturn]] void Throw(except::Type type,
                        std::source_location = std::source_location::current());
void Rethrow();
void FinishTry();

}  // namespace except::detail
