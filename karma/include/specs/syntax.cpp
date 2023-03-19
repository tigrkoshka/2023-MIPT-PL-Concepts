#include "syntax.hpp"

#include <cctype>

namespace karma::detail::specs::syntax {

bool IsAllowedLabelChar(char symbol) {
    return std::islower(symbol) != 0 ||  //
           std::isdigit(symbol) != 0 ||  //
           symbol == '_' ||              //
           symbol == '.';
}

}  // namespace karma::detail::specs::syntax
