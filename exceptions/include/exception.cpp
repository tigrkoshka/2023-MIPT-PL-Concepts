#include "exception.hpp"

#include <string>  // for string

namespace except {

std::string Message(Type type) {
    switch (type) {
        case Type::A:
            return "A";
        case Type::B:
            return "B";
        case Type::C:
            return "C";
        case Type::D:
            return "D";
    }
}

}  // namespace except
