#pragma once

#include <string>  // for string

namespace karma::detail::utils::strings {

void TrimSpaces(std::string& str);

void Unescape(std::string& str);

}  // namespace karma::detail::utils::strings