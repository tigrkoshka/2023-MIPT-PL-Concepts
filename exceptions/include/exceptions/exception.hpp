#pragma once

#include <source_location>  // for source_location
#include <string>           // for string

namespace except {

struct Exception {
    virtual ~Exception() = default;
};

}  // namespace except
