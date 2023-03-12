#pragma once

#include <string>  // for string

namespace karma::detail::specs::syntax {

constexpr char8_t kCommentSep        = '#';
constexpr char8_t kDisableCommentSep = '\\';
constexpr char8_t kLabelEnd          = ':';

bool IsAllowedLabelChar(char);

const std::string kIncludeDirective    = "include";
const std::string kEntrypointDirective = "end";

}  // namespace karma::detail::specs::syntax
