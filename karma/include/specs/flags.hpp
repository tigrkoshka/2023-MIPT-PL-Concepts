#pragma once

#include "specs/architecture.hpp"

namespace karma::detail::specs::flags {

enum Flag : arch::Register {
    EQUAL            = 0b1,
    NOT_EQUAL        = 0b10,
    GREATER          = 0b100,
    LESS             = 0b1000,
    GREATER_OR_EQUAL = 0b10000,
    LESS_OR_EQUAL    = 0b100000,
};

constexpr arch::Register kEqual   = EQUAL | GREATER_OR_EQUAL | LESS_OR_EQUAL;
constexpr arch::Register kGreater = NOT_EQUAL | GREATER | GREATER_OR_EQUAL;
constexpr arch::Register kLess    = NOT_EQUAL | LESS | LESS_OR_EQUAL;

}  // namespace karma::detail::specs::flags
