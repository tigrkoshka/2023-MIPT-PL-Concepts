#pragma once

#include <iostream>
#include <type_traits>  // for is_destructible_v, ...

#include "manager.hpp"

template <typename T>
    requires(std::is_class_v<T> &&   //
             !std::is_final_v<T> &&  //
             std::same_as<std::decay_t<T>, T>)
struct AutoObject final : T {
   private:
    static constexpr bool kObjectManagerEnabled = std::is_destructible_v<T>;

   private:
    template <typename To>
    static void Test(To) {}

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
    // default constructor (if T supports it)
    AutoObject()  //
        noexcept(std::is_nothrow_default_constructible_v<T> &&
                 !kObjectManagerEnabled)
        requires std::is_default_constructible_v<T>
        : T() {
        std::cout << "default\n";
        Register();
    };

    // propagate explicit single-argument constructors of T
    //
    //  [NOTE]: the constraint !std::same_as<std::decay_t<From>, T>
    //          prevents this constructor from hiding cope or move
    //          constructors
    //
    //  TODO: the linter bugprone-forwarding-reference-overload does not
    //        work with concepts yet, so it creates a false positive report
    //        despite the note above, so we explicitly disable it
    template <typename From>
        requires(!std::same_as<std::decay_t<From>, T> &&
                 std::is_constructible_v<T, From> &&
                 !std::is_convertible_v<From, T>)
    // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
    explicit AutoObject(From&& from)  //
        noexcept(std::is_nothrow_constructible_v<T, From> &&
                 !kObjectManagerEnabled)
        : T(std::forward<From>(from)) {
        std::cout << "explicit single-argument\n";
        Register();
    }

    // propagate implicit single-argument constructors of T
    //
    //  [NOTE]: this is more constrained than the previous one
    //          (notice the std::is_convertible_v<From, T> requirement),
    //          so it will be chosen if possible and no ambiguity
    //          will occur
    //
    //  [NOTE]: the constraint !std::same_as<std::decay_t<From>, T>
    //          prevents this constructor from hiding cope or move
    //          constructors
    //
    //  TODO: the linter bugprone-forwarding-reference-overload does not
    //        work with concepts yet, so it creates a false positive report
    //        despite the note above, so we explicitly disable it
    template <typename From>
        requires(!std::same_as<std::decay_t<From>, T> &&
                 std::is_constructible_v<T, From> &&
                 std::is_convertible_v<From, T>)
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    AutoObject(From&& from)  // NOLINT(bugprone-forwarding-reference-overload)
        noexcept(std::is_nothrow_constructible_v<T, From> &&
                 !kObjectManagerEnabled)
        : T(std::forward<From>(from)) {
        std::convertible_to<size_t, bool>;
        static_assert(std::is_convertible_v<From, T>);
        std::cout << "implicit single-argument\n";
        Register();
    }

    // propagate explicit multi-argument constructors of T
    template <typename First, typename Second, typename... Rest>
        requires(std::is_constructible_v<T, First, Second, Rest...> &&
                 !requires {
                     {
                         Test<T>({std::declval<First>(),
                                  std::declval<Second>(),
                                  std::declval<Rest>()...})
                     };
                 })
    explicit AutoObject(First&& first, Second&& second, Rest&&... rest)  //
        noexcept(std::is_nothrow_constructible_v<T, First, Second, Rest...> &&
                 !kObjectManagerEnabled)
        : T(std::forward<First>(first),
            std::forward<Second>(second),
            std::forward<Rest>(rest)...) {
        std::cout << "explicit multi-argument\n";
        Register();
    }

    // propagate implicit multi-argument constructors of T
    //
    //  [NOTE]: this is more constrained than the previous one
    //          (notice the additional requirement using the Test function),
    //          so it will be chosen if possible and no ambiguity
    //          will occur
    template <typename First, typename Second, typename... Rest>
        requires(std::is_constructible_v<T, First, Second, Rest...> &&
                 requires {
                     {
                         Test<T>({std::declval<First>(),
                                  std::declval<Second>(),
                                  std::declval<Rest>()...})
                     };
                 })
    AutoObject(First&& first, Second&& second, Rest&&... rest)  //
        noexcept(std::is_nothrow_constructible_v<T, First, Second, Rest...> &&
                 !kObjectManagerEnabled)
        : T(std::forward<First>(first),
            std::forward<Second>(second),
            std::forward<Rest>(rest)...) {
        std::cout << "implicit multi-argument\n";
        Register();
    }

    // copy constructor (if T supports it)

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    AutoObject(const T& instance)  //
        noexcept(std::is_nothrow_copy_constructible_v<T> &&
                 !kObjectManagerEnabled)
        requires std::is_copy_constructible_v<T>
        : T(instance) {
        std::cout << "copy from T\n";
        Register();
    }

    AutoObject(const AutoObject& other)  //
        noexcept(std::is_nothrow_copy_constructible_v<T> &
                 !kObjectManagerEnabled)
        requires std::is_copy_constructible_v<T>
        : T(static_cast<const T&>(other)) {
        std::cout << "copy from AutoObject\n";
        Register();
    }

    // move constructor (if T supports it)

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    AutoObject(T&& instance)  //
        noexcept(std::is_nothrow_move_constructible_v<T> &&
                 !kObjectManagerEnabled)
        requires std::is_move_constructible_v<T>
        : T(std::move(instance)) {
        std::cout << "move from T\n";
        Register();
    }

    AutoObject(AutoObject&& other)  //
        noexcept(std::is_nothrow_move_constructible_v<T> &&
                 !kObjectManagerEnabled)
        requires std::is_move_constructible_v<T>
        : T(static_cast<T&&>(other)) {
        std::cout << "move from AutoObject\n";
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

    // move assignment (if T supports it)
    AutoObject& operator=(AutoObject&&)  //
        noexcept(std::is_nothrow_move_assignable_v<T> && !kObjectManagerEnabled)
        requires std::is_move_assignable_v<T>
    = default;
};
