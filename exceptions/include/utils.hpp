#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

#include "exception.hpp"

namespace except::details {

void Push(Node* exception);
void Pop();
void Throw(Type type, std::string file, size_t line);

}  // namespace except::details
