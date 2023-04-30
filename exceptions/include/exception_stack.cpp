#include "exception_stack.hpp"

#include <optional>  // for optional, nullopt
#include <stack>     // for stack

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

void Node::Raise(Type type, const char* file, size_t line) {
    exception_.type = type;
    exception_.file = file;
    exception_.line = line;
    status_ = Status::RAISED;
}

bool Node::Handle(std::optional<Type> type) {
    if (type.has_value() && exception_.type != type.value()) {
        return false;
    }

    status_ = Status::HANDLED;
    return true;
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
            Throw(exception_.type, exception_.file, exception_.line);
    }
}

bool Node::IsFinalized() const {
    return finalized_;
}

std::optional<Node*> Node::TryGetCurrent() {
    if (stack.empty()) {
        return std::nullopt;
    }

    Node* node = stack.top();
    stack.pop();

    return node;
}

}  // namespace except::detail
