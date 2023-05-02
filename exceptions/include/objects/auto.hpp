#pragma once

#include <iostream>
#include <type_traits>  // for is_destructible_v, ...

#include "manager.hpp"
#include "utils/concepts.hpp"

namespace except::detail {

template <typename T>
    requires(std::is_class_v<T> &&   //
             !std::is_final_v<T> &&  //
             std::same_as<std::decay_t<T>, T>)
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
    // general form constructors

    template <typename... From>
        requires utils::concepts::IsOnlyExplicitlyConstructible<T, From...>
    explicit AutoObject(From&&... from)  //
        noexcept(std::is_nothrow_constructible_v<T, From...> &&
                 !kObjectManagerEnabled)
        : T(std::forward<From>(from)...) {
//        std::cout << "AutoObject: explicit\n";
        Register();
    }

    template <typename... From>
        requires utils::concepts::IsImplicitlyConstructible<T, From...>
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    AutoObject(From&&... from)  //
        noexcept(std::is_nothrow_constructible_v<T, From...> &&
                 !kObjectManagerEnabled)
        : T(std::forward<From>(from)...) {
//        std::cout << "AutoObject: implicit\n";
        Register();
    }

    // initializer list constructors

    template <typename List>
        requires utils::concepts::IsOnlyExplicitlyConstructible<T, std::initializer_list<List>&>
    explicit AutoObject(std::initializer_list<List> list)  //
        noexcept(
            std::is_nothrow_constructible_v<T, std::initializer_list<List>> &&
            !kObjectManagerEnabled)
        : T(list) {
//        std::cout << "AutoObject: explicit initializer list\n";
        Register();
    }

    template <typename List>
        requires utils::concepts::IsImplicitlyConstructible<T, std::initializer_list<List>&>
    AutoObject(std::initializer_list<List> list)  //
        noexcept(
            std::is_nothrow_constructible_v<T, std::initializer_list<List>> &&
            !kObjectManagerEnabled)
        : T(list) {
//        std::cout << "AutoObject: implicit initializer list\n";
        Register();
    }

    // initializer list with additional arguments constructors

    template <typename List, typename... Args>
        requires utils::concepts::IsOnlyExplicitlyConstructible<T, std::initializer_list<List>&, Args...>
    explicit AutoObject(std::initializer_list<List> list, Args... args)  //
        noexcept(std::is_nothrow_constructible_v<T,
                                                 std::initializer_list<List>&,
                                                 Args...> &&
                 !kObjectManagerEnabled)
        : T(list, std::forward<Args>(args)...) {
//        std::cout << "AutoObject: explicit initializer list\n";
        Register();
    }

    template <typename List, typename... Args>
        requires utils::concepts::IsImplicitlyConstructible<T, std::initializer_list<List>&, Args...>
    AutoObject(std::initializer_list<List> list, Args... args)  //
        noexcept(std::is_nothrow_constructible_v<T,
                                                 std::initializer_list<List>&,
                                                 Args...> &&
                 !kObjectManagerEnabled)
        : T(list, std::forward<Args>(args)...) {
//        std::cout << "AutoObject: implicit initializer list\n";
        Register();
    }

    // copy constructor

    AutoObject(const AutoObject& other)  //
        noexcept(std::is_nothrow_copy_constructible_v<T> &
                 !kObjectManagerEnabled)
        requires(std::is_copy_constructible_v<T> &&
                 !std::is_constructible_v<T, const AutoObject&>)
        : T(static_cast<const T&>(other)) {
//        std::cout << "AutoObject: copy from AutoObject\n";
        Register();
    }

    // move constructor

    AutoObject(AutoObject&& other)  //
        noexcept(std::is_nothrow_move_constructible_v<T> &&
                 !kObjectManagerEnabled)
        requires(std::is_move_constructible_v<T> &&
                 !std::is_constructible_v<T, AutoObject &&>)
        : T(static_cast<T&&>(other)) {
//        std::cout << "AutoObject: move from AutoObject\n";
        Register();
    }

    // destructor
    //
    //   [NOTE]: the destructor in this case is never noexcept,
    //           because even if T is nothrow destructible, then we
    //           perform actions with the Manager's stack, which
    //           may throw
    ~AutoObject() noexcept(false) {
        Unregister();
    }

    // copy assignment (if T supports it)

    AutoObject& operator=(const AutoObject&)  //
        noexcept(std::is_nothrow_copy_assignable_v<T> && !kObjectManagerEnabled)
        requires std::is_copy_assignable_v<T>
    = default;

    // NOLINTNEXTLINE(fuchsia-overloaded-operator)
    AutoObject& operator=(const T& other)  //
        noexcept(std::is_nothrow_copy_assignable_v<T> && !kObjectManagerEnabled)
        requires std::is_copy_assignable_v<T>
    {
        static_cast<T*>(this)->operator=(other);
        return *this;
    };

    // move assignment (if T supports it)

    AutoObject& operator=(AutoObject&&)  //
        noexcept(std::is_nothrow_move_assignable_v<T> && !kObjectManagerEnabled)
        requires std::is_move_assignable_v<T>
    = default;

    // NOLINTNEXTLINE(fuchsia-overloaded-operator)
    AutoObject& operator=(T&& other)  //
        noexcept(std::is_nothrow_move_assignable_v<T> && !kObjectManagerEnabled)
        requires std::is_move_assignable_v<T>
    {
        static_cast<T*>(this)->operator=(std::move(other));
        return *this;
    };
};

}  // namespace except::detail

template <typename T>
using AutoObject = except::detail::AutoObject<T>;
