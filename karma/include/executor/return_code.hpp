#pragma once

#include <cstdint> // for uint32_t
#include <optional> // for optional

namespace karma::executor::detail {

using ReturnCode = uint32_t;
using MaybeReturnCode = std::optional<ReturnCode>;

}