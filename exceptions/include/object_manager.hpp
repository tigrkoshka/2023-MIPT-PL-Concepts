#pragma once

#include <functional>   // for function
#include <stack>        // for stack
#include <tuple>        // for tuple
#include <type_traits>  // for is_destructible_v
#include <variant>      // for variant, monostate

namespace except::details {

class ObjectManager final {
   private:
    // cannot use std::destructible, because we need to allow
    // types to be destructible, but not nothrow_destructible
    // since in user-defined classes destructors are often
    // not marked noexcept (even if they could be)
    template <typename T>
        requires std::is_destructible_v<T>
    struct Destroyer {
        // NOLINTNEXTLINE(fuchsia-overloaded-operator)
        void operator()(void* ptr) const {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            reinterpret_cast<T*>(ptr)->~T();
        }
    };

    // { destructor, instance }
    using Destructible = std::tuple<std::function<void(void*)>, void*>;
    using Checkpoint   = std::monostate;

    using Item = std::variant<Checkpoint, Destructible>;

   private:
    static bool TopIsCheckpoint();
    static void DestroyTop();

   public:
    ObjectManager() = delete;

    template <typename T>
    static void RecordObject(T& instance) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        void* ptr = reinterpret_cast<void*>(&instance);
        k_destroyers.emplace(std::make_tuple(Destroyer<T>(), ptr));
    }

    static void RecordCheckpoint();

    // TODO: merge the following two methods
    static void PopCheckpoint();
    static void PopDestructible();

    static void UnwindToCheckpoint();

   private:
    static std::stack<Item> k_destroyers;
};

}  // namespace except::details
