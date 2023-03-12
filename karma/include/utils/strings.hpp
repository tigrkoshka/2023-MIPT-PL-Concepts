#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

namespace karma::detail::utils::strings {

void TrimSpaces(std::string& str);

// TODO: docs
std::string Escape(const std::string& str,
                   size_t start = 0,
                   size_t end   = std::string::npos);

// TODO: docs
void Unescape(std::string& str,
              size_t start = 0,
              size_t end   = std::string::npos);

}  // namespace karma::detail::utils::strings