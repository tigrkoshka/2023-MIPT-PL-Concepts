#pragma once

#include <stack>        // for stack
#include <type_traits>  // for is_destructible_v
#include <utility>      // for pair
#include <variant>      // for variant, monostate

#include "utils/concepts.hpp"

namespace except::detail {

class ObjectManager final {
   private:
    // cannot use std::destructible, because we need to allow
    // types to be destructible, but not nothrow_destructible
    // since in user-defined classes destructors are often
    // not marked noexcept (even if they could be)
    template <typename T>
        requires utils::concepts::Class<T> &&  //
                 utils::concepts::NonCV<T> &&  //
                 std::is_destructible_v<T>
    static void Destroy(void* ptr) {
        static_cast<T*>(ptr)->~T();
    }

    // Checkpoint or destroyer with argument
    using Item =
        std::variant<std::monostate, std::pair<void (*)(void*), void*>>;

   public:
    // static class
    ObjectManager() = delete;

    template <typename T>
        requires utils::concepts::Class<T> &&  //
                 utils::concepts::NonCV<T> &&  //
                 std::is_destructible_v<T>
    // NOLINTNEXTLINE(bugprone-exception-escape)
    static void RecordObject(T& instance) noexcept {
        k_destroyers.emplace(std::make_pair(Destroy<T>, &instance));
    }
    static void RecordCheckpoint();

    static void Pop() noexcept;

    static void UnwindToCheckpoint();

   private:
    static std::stack<Item> k_destroyers;
};

template <typename T>
void MaybeRecordObject(const T& instance) {
    if constexpr (std::is_destructible_v<T>) {
        ObjectManager::RecordObject(instance);
    }
}

template <typename T>
void MaybePopObject() {
    if constexpr (std::is_destructible_v<T>) {
        ObjectManager::Pop();
    }
}

}  // namespace except::detail
