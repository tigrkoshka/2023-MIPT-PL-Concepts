#include "node.hpp"

#include <cstddef>          // for size_t
#include <source_location>  // for source_location

#include "exceptions/exception.hpp"

namespace except::detail {

Node::Node(size_t depth)
    : depth(depth) {}

int* Node::Buff() {
    return static_cast<int*>(buf);
}

void Node::Raise(Type type, std::source_location source_location) {
    exception.type            = type;
    exception.source_location = source_location;
    raised                    = true;
}

}  // namespace except::detail
