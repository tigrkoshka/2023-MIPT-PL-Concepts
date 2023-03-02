#pragma once

#include "architecture.hpp"

namespace karma::detail::specs::flags {

enum Flag : arch::Word {
    EQUAL            = 0b1u,
    NOT_EQUAL        = 0b10u,
    GREATER          = 0b100u,
    LESS             = 0b1000u,
    GREATER_OR_EQUAL = 0b10000u,
    LESS_OR_EQUAL    = 0b100000u,
};

static const arch::Word kEqual   = EQUAL + GREATER_OR_EQUAL + LESS_OR_EQUAL;
static const arch::Word kGreater = NOT_EQUAL + GREATER + GREATER_OR_EQUAL;
static const arch::Word kLess    = NOT_EQUAL + LESS + LESS_OR_EQUAL;

}  // namespace karma::detail::specs::flags
