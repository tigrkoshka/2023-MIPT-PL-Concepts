#include "syntax.hpp"

#include <cctype>

bool IsAllowedLabelChar(char symbol) {
    return std::islower(symbol) != 0 ||  //
           std::isdigit(symbol) != 0 ||  //
           symbol == '_' ||              //
           symbol == '.';
}