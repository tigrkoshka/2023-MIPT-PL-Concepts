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

void Node::Raise(Data exc, std::source_location srcl) {
    data            = std::move(exc);
    source_location = srcl;
    raised          = true;
}

}  // namespace except::detail
