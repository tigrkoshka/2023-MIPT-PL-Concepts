#pragma once

#include <stack>        // for stack
#include <type_traits>  // for is_destructible_v
#include <utility>      // for pair
#include <variant>      // for variant, monostate

namespace except::detail {

class ObjectManager final {
   private:
    // cannot use std::destructible, because we need to allow
    // types to be destructible, but not nothrow_destructible
    // since in user-defined classes destructors are often
    // not marked noexcept (even if they could be)
    template <typename T>
        requires std::is_destructible_v<T>
    static void Destroy(void* ptr) {
        static_cast<T*>(ptr)->~T();
    }

    using Destroyer = void(void*);

    // Checkpoint or destroyer with argument
    using Item = std::variant<std::monostate, std::pair<Destroyer*, void*>>;

   public:
    // static class
    ObjectManager() = delete;

    template <typename T>
        requires std::is_destructible_v<T>
    static void RecordObject(T& instance) {
        k_destroyers.emplace(std::make_pair(Destroy<T>, &instance));
    }
    static void RecordCheckpoint();

    // TODO: merge the following two methods
    static void PopDestructible();
    static void PopCheckpoint();

    static void UnwindToCheckpoint();

   private:
    static std::stack<Item> k_destroyers;
};

}  // namespace except::detail
