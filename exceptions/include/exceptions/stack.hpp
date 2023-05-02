#pragma once

#include <csetjmp>          // for jump_buf
#include <optional>         // for optional
#include <source_location>  // for source_location
#include <utility>          // for pair

#include "exceptions/exception.hpp"

namespace except::detail {

std::pair<bool, int*> StartTry();
bool Catch();
bool TryCatch(Type);
void FinishTry();

[[noreturn]] void Throw(except::Type type,
                        std::source_location = std::source_location::current());
[[noreturn]] void Rethrow();

}  // namespace except::detail
