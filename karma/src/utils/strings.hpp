#pragma once

#include <string>  // for string

namespace karma::detail::utils::strings {

void TrimSpaces(std::string& str);

std::string Escape(const std::string& str,
                   size_t start = 0,
                   size_t end   = std::string::npos);

void Unescape(std::string& str,
              size_t start = 0,
              size_t end   = std::string::npos);

}  // namespace karma::detail::utils::strings