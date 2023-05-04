#pragma once

#include <concepts>          // for constructible_from, ...
#include <initializer_list>  // for initializer_list
#include <type_traits>       // for is_destructible_v, ...
#include <utility>           // for forward

#include "objects/manager.hpp"
#include "utils/concepts.hpp"

namespace except::detail {

template <typename T>
    requires(utils::concepts::Class<T> &&  //
             utils::concepts::NonCVRef<T>)
struct AutoObject final {
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
        except::detail::ObjectManager::Pop();
    }

   public:
    // general form constructors

    template <typename... From>
        requires std::constructible_from<T, From...>
    explicit(!utils::concepts::ImplicitlyConstructibleFrom<T, From...>)
        // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
        AutoObject(From&&... from)  //
        noexcept(std::is_nothrow_constructible_v<T, From...> &&
                 !kObjectManagerEnabled)
        : instance_(std::forward<From>(from)...) {
        Register();
    }

    // initializer list constructor

    template <typename List>
        requires std::constructible_from<T, std::initializer_list<List>&>
    explicit(!utils::concepts::ImplicitlyConstructibleFrom<
             T,
             std::initializer_list<List>&>)
        AutoObject(std::initializer_list<List> list)  //
        noexcept(
            std::is_nothrow_constructible_v<T, std::initializer_list<List>> &&
            !kObjectManagerEnabled)
        : instance_(list) {
        Register();
    }

    // initializer list with additional arguments constructor

    template <typename List, typename... Args>
        requires std::
            constructible_from<T, std::initializer_list<List>&, Args...>
        explicit(!utils::concepts::ImplicitlyConstructibleFrom<
                 T,
                 std::initializer_list<List>&,
                 Args...>)
            AutoObject(std::initializer_list<List> list, Args... args)  //
        noexcept(std::is_nothrow_constructible_v<T,
                                                 std::initializer_list<List>&,
                                                 Args...> &&
                 !kObjectManagerEnabled)
        : instance_(list, std::forward<Args>(args)...) {
        Register();
    }

    // copy constructor

    explicit(!utils::concepts::ImplicitlyConstructibleFrom<T, const T&>)
        AutoObject(const AutoObject& other)  //
        noexcept(std::is_nothrow_copy_constructible_v<T> &
                 !kObjectManagerEnabled)
        requires(std::is_copy_constructible_v<T> &&
                 !std::is_constructible_v<T, const AutoObject&>)
        : instance_(static_cast<const T&>(other)) {
        Register();
    }

    // move constructor

    explicit(!utils::concepts::ImplicitlyConstructibleFrom<T, T&&>)
        AutoObject(AutoObject&& other)  //
        noexcept(std::is_nothrow_move_constructible_v<T> &&
                 !kObjectManagerEnabled)
        requires(std::is_move_constructible_v<T> &&
                 !std::is_constructible_v<T, AutoObject &&>)
        : instance_(static_cast<T&&>(other)) {
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
        instance_ = other;
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
        instance_ = std::move(other);
        return *this;
    };

    // access

    // NOLINTNEXTLINE(fuchsia-overloaded-operator)
    T* operator->() noexcept {
        return &instance_;
    }

    // NOLINTNEXTLINE(fuchsia-overloaded-operator)
    const T* operator->() const noexcept {
        return &instance_;
    }

    T& Get() noexcept {
        return instance_;
    }

    const T& Get() const noexcept {
        return instance_;
    }

   private:
    T instance_;
};

template <typename T>
    requires(utils::concepts::Class<T> &&     //
             utils::concepts::NonCVRef<T> &&  //
             !std::is_final_v<T>)
struct AutoObject<T> final : T {
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
        except::detail::ObjectManager::Pop();
    }

   public:
    // general form constructors

    template <typename... From>
        requires std::constructible_from<T, From...>
    explicit(!utils::concepts::ImplicitlyConstructibleFrom<T, From...>)
        // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
        AutoObject(From&&... from)  //
        noexcept(std::is_nothrow_constructible_v<T, From...> &&
                 !kObjectManagerEnabled)
        : T(std::forward<From>(from)...) {
        Register();
    }

    // initializer list constructor

    template <typename List>
        requires std::constructible_from<T, std::initializer_list<List>&>
    explicit(!utils::concepts::ImplicitlyConstructibleFrom<
             T,
             std::initializer_list<List>&>)
        AutoObject(std::initializer_list<List> list)  //
        noexcept(
            std::is_nothrow_constructible_v<T, std::initializer_list<List>> &&
            !kObjectManagerEnabled)
        : T(list) {
        Register();
    }

    // initializer list with additional arguments constructor

    template <typename List, typename... Args>
        requires std::
            constructible_from<T, std::initializer_list<List>&, Args...>
        explicit(!utils::concepts::ImplicitlyConstructibleFrom<
                 T,
                 std::initializer_list<List>&,
                 Args...>)
            AutoObject(std::initializer_list<List> list, Args... args)  //
        noexcept(std::is_nothrow_constructible_v<T,
                                                 std::initializer_list<List>&,
                                                 Args...> &&
                 !kObjectManagerEnabled)
        : T(list, std::forward<Args>(args)...) {
        Register();
    }

    // copy constructor

    explicit(!utils::concepts::ImplicitlyConstructibleFrom<T, const T&>)
        AutoObject(const AutoObject& other)  //
        noexcept(std::is_nothrow_copy_constructible_v<T> &
                 !kObjectManagerEnabled)
        requires(std::is_copy_constructible_v<T> &&
                 !std::is_constructible_v<T, const AutoObject&>)
        : T(static_cast<const T&>(other)) {
        Register();
    }

    // move constructor

    explicit(!utils::concepts::ImplicitlyConstructibleFrom<T, T&&>)
        AutoObject(AutoObject&& other)  //
        noexcept(std::is_nothrow_move_constructible_v<T> &&
                 !kObjectManagerEnabled)
        requires(std::is_move_constructible_v<T> &&
                 !std::is_constructible_v<T, AutoObject &&>)
        : T(static_cast<T&&>(other)) {
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
