#pragma once

#include <string>  // for string

namespace karma::detail::specs::syntax {

constexpr char kCommentSep        = '#';
constexpr char kDisableCommentSep = '\\';
constexpr char kLabelEnd          = ':';

bool IsAllowedLabelChar(char symbol);

const std::string kIncludeDirective    = "include";
const std::string kEntrypointDirective = "end";

}  // namespace karma::detail::specs::syntax
