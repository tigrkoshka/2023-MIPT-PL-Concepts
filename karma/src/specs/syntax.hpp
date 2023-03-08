#pragma once

#include <string>         // for string

namespace karma::detail::specs::syntax {

static const char8_t kCommentSep = ';';
static const char8_t kLabelEnd   = ':';

bool IsAllowedLabelChar(char);

static const std::string kEntrypointDirective = "end";

}  // namespace karma::detail::specs::syntax
