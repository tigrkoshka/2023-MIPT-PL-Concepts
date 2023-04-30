#pragma once

#include <type_traits>  // for is_destructible_v, ...

#include "object_manager.hpp"

template <typename T>
struct AutoObject final : T {
   private:
    static constexpr bool kObjectManagerEnabled = std::is_destructible_v<T>;

   private:
    void Register() noexcept {}
    void Unregister() noexcept {}

    void Register()
        requires kObjectManagerEnabled
    {
        except::detail::ObjectManager::RecordObject<AutoObject<T>>(*this);
    }

    void Unregister()
        requires kObjectManagerEnabled
    {
        except::detail::ObjectManager::PopDestructible();
    }

   public:
    AutoObject()  //
        noexcept(std::is_nothrow_default_constructible_v<T> &&
                 !kObjectManagerEnabled)
        requires std::is_default_constructible_v<T>
        : T() {
        Register();
    };

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    AutoObject(const T& instance)  //
        noexcept(std::is_nothrow_copy_constructible_v<T> &&
                 !kObjectManagerEnabled)
        requires std::is_copy_constructible_v<T>
        : T(instance) {
        Register();
    }

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    AutoObject(T&& instance)  //
        noexcept(std::is_nothrow_move_constructible_v<T> &&
                 !kObjectManagerEnabled)
        requires std::is_move_constructible_v<T>
        : T(std::move(instance)) {
        Register();
    }

    AutoObject(const AutoObject& other)  //
        noexcept(std::is_nothrow_copy_constructible_v<T> &
                 !kObjectManagerEnabled)
        requires std::is_copy_constructible_v<T>
        : T(static_cast<const T&>(other)) {
        Register();
    }

    AutoObject(AutoObject&& other)  //
        noexcept(std::is_nothrow_move_constructible_v<T> &&
                 !kObjectManagerEnabled)
        requires std::is_move_constructible_v<T>
        : T(static_cast<T&&>(other)) {
        Register();
    }

    ~AutoObject() noexcept(false) {
        Unregister();
    }

    AutoObject& operator=(const AutoObject&)  //
        noexcept(std::is_nothrow_copy_assignable_v<T> && !kObjectManagerEnabled)
        requires std::is_copy_assignable_v<T>
    = default;

    AutoObject& operator=(AutoObject&&)  //
        noexcept(std::is_nothrow_move_assignable_v<T> && !kObjectManagerEnabled)
        requires std::is_move_assignable_v<T>
    = default;
};
