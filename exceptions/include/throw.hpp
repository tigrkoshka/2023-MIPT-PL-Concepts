#pragma once

#include <source_location>  // for source_location

#include "exception.hpp"

void Throw(except::Type type,
           std::source_location = std::source_location::current());
