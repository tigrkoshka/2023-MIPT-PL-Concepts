#include "exception_stack.hpp"

#include <csetjmp>          // for longjmp
#include <iostream>         // for cerr, endl
#include <optional>         // for optional, nullopt
#include <source_location>  // for source_location
#include <stack>            // for stack

#include "object_manager.hpp"
#include "throw.hpp"

namespace except::detail {

thread_local std::stack<Node*> stack;

Node::Node() {
    stack.push(this);
    ObjectManager::RecordCheckpoint();
}

int* Node::Buff() {
    return static_cast<int*>(buf_);
}

void Node::Raise(Type type, std::source_location source_location) {
    exception_.type            = type;
    exception_.source_location = source_location;
    status_                    = Status::RAISED;
}

bool Node::Handle(std::optional<Type> type) {
    if (type.has_value() && exception_.type != type.value()) {
        return false;
    }

    status_ = Status::HANDLED;
    return true;
}

void Node::Rethrow() const {
    Throw(exception_.type, exception_.source_location);
}

void Node::Finalize() {
    finalized_ = true;

    switch (status_) {
        case Status::NO_EXCEPTION:
            ObjectManager::PopCheckpoint();
            stack.pop();
            break;
        case Status::HANDLED:
            // do nothing
            break;
        case Status::RAISED:
            Rethrow();
    }
}

bool Node::IsFinalized() const {
    return finalized_;
}

std::optional<Node*> TryGetCurrent() {
    if (stack.empty()) {
        return std::nullopt;
    }

    Node* node = stack.top();
    stack.pop();

    return node;
}

}  // namespace except::detail
