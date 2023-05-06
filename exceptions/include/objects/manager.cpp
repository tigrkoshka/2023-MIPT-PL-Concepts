#include "manager.hpp"

#include <stack>    // for stack
#include <variant>  // for get

namespace except::detail {

void ObjectManager::RecordCheckpoint() {
    k_destroyers.emplace();
}

void ObjectManager::Pop() noexcept {
    k_destroyers.pop();
}

void ObjectManager::UnwindToCheckpoint() {
    while (k_destroyers.top().index() == 1) {
        auto& [destroy, instance] = std::get<1>(k_destroyers.top());
        destroy(instance);
        // we do not explicitly call k_destructors.pop() since the class
        // of the instance must be derived from AutoObject and therefore
        // calls k_destructors.pop() on destruction (see auto_object.hpp)
    }

    k_destroyers.pop();
}

std::stack<ObjectManager::Item> ObjectManager::k_destroyers;

}  // namespace except::detail
