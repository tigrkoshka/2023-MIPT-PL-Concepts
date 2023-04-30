#include "object_manager.hpp"

#include <stack>  // for stack

namespace except::detail {

bool ObjectManager::TopIsCheckpoint() {
    return !k_destroyers.empty() && k_destroyers.top().index() == 0;
}

void ObjectManager::DestroyTop() {
    auto& [destroy, instance] = std::get<1>(k_destroyers.top());
    destroy(instance);
    // we do not explicitly call k_destructors.pop() since the class
    // of the instance must be derived from AutoObject and therefore
    // calls k_destructors.pop() on destruction (see auto_object.hpp)
}

void ObjectManager::RecordCheckpoint() {
    k_destroyers.emplace();
}

void ObjectManager::PopCheckpoint() {
    // TODO: this line is for debug
    //       (to throw an exception if anything unexpected happens)
    //       delete it before submitting the task
    assert(k_destroyers.top().index() == 0);
    k_destroyers.pop();
}

void ObjectManager::PopDestructible() {
    // TODO: this line is for debug
    //       (to throw an exception if anything unexpected happens)
    //       delete it before submitting the task
    assert(k_destroyers.top().index() == 1);
    k_destroyers.pop();
}

void ObjectManager::UnwindToCheckpoint() {
    while (!TopIsCheckpoint()) {
        DestroyTop();
    }
    PopCheckpoint();
}

std::stack<ObjectManager::Item> ObjectManager::k_destroyers;

}  // namespace except::detail
