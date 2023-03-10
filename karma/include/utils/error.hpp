#pragma once

#include <stdexcept>  // for runtime_error
#include <string>     // for string

namespace karma::errors {

struct Error : std::runtime_error {
   protected:
    explicit Error(const std::string& message)
        : std::runtime_error(message) {}
};

}  // namespace karma::errors
