#pragma once

#include <initializer_list>  // for initializer_list
#include <iostream>
#include <type_traits>  // for is_destructible_v, ...
#include <utility>      // for forward

#include "objects/manager.hpp"
#include "utils/concepts.hpp"

#define OVERLOAD_UNARY_OPERATOR(op)                 \
    decltype(auto) operator op() /**/               \
        noexcept(noexcept(instance_.operator op())) \
        requires requires(T instance) {             \
            { instance.operator op() };             \
        }                                           \
    {                                               \
        return instance_.operator op();             \
    }                                               \
                                                    \
    decltype(auto) operator op() const /**/         \
        noexcept(noexcept(instance_.operator op())) \
        requires requires(const T instance) {       \
            { instance.operator op() };             \
        }                                           \
    {                                               \
        return instance_.operator op();             \
    }

#define OVERLOAD_BINARY_OPERATOR(op)                                    \
    template <typename U>                                               \
    decltype(auto) operator op(U&& rhs) /**/                            \
        noexcept(noexcept(instance_.operator op(std::forward<U>(rhs)))) \
        requires requires(T instance) {                                 \
            { instance.operator op(std::forward<U>(rhs)) };             \
        }                                                               \
    {                                                                   \
        return instance_.operator op(std::forward<U>(rhs));             \
    }                                                                   \
                                                                        \
    template <typename U>                                               \
    decltype(auto) operator op(U&& rhs) const /**/                      \
        noexcept(noexcept(instance_.operator op(std::forward<U>(rhs)))) \
        requires requires(const T instance) {                           \
            { instance.operator op(std::forward<U>(rhs)) };             \
        }                                                               \
    {                                                                   \
        return instance_.operator op(std::forward<U>(rhs));             \
    }

#define OVERLOAD_MULTI_ARGUMENT_OPERATOR(op)                                   \
    template <typename... Args>                                                \
    decltype(auto) operator op(Args&&... args) /**/                            \
        noexcept(noexcept(instance_.operator op(std::forward<Args>(args)...))) \
        requires requires(T instance) {                                        \
            { instance.operator op(std::forward<Args>(args)...) };             \
        }                                                                      \
    {                                                                          \
        return instance_.operator op(std::forward<Args>(args)...);             \
    }                                                                          \
                                                                               \
    template <typename... Args>                                                \
    decltype(auto) operator op(Args&&... args) const /**/                      \
        noexcept(noexcept(instance_.operator op(std::forward<Args>(args)...))) \
        requires requires(const T instance) {                                  \
            { instance.operator op(std::forward<Args>(args)...) };             \
        }                                                                      \
    {                                                                          \
        return instance_.operator op(std::forward<Args>(args)...);             \
    }

#define OVERLOAD_ALLOCATION(op)                                                \
    static void* operator op(size_t count) /**/                                \
        noexcept(noexcept(T::operator op(count)))                              \
        requires requires {                                                    \
            { T::operator op(count) };                                         \
        }                                                                      \
    {                                                                          \
        return T::operator op(count);                                          \
    }                                                                          \
                                                                               \
    static void* operator op(size_t count, std::align_val_t align) /**/        \
        noexcept(noexcept(T::operator op(count, align)))                       \
        requires requires {                                                    \
            { T::operator op(count, align) };                                  \
        }                                                                      \
    {                                                                          \
        return T::operator op(count, align);                                   \
    }                                                                          \
                                                                               \
    template <typename First, typename... Rest>                                \
    static void* operator op(size_t count, First&& first, Rest&&... rest) /**/ \
        noexcept(noexcept(T::operator op(count,                                \
                                         std::forward<First>(first),           \
                                         std::forward<Rest>(rest)...)))        \
        requires(!std::is_same_v<std::align_val_t, First> &&                   \
                 requires {                                                    \
                     {                                                         \
                         T::operator op(count,                                 \
                                        std::forward<First>(first),            \
                                        std::forward<Rest>(rest)...)           \
                     };                                                        \
                 })                                                            \
    {                                                                          \
        return T::operator op(count,                                           \
                              std::forward<First>(first),                      \
                              std::forward<Rest>(rest)...);                    \
    }                                                                          \
                                                                               \
    template <typename First, typename... Rest>                                \
    static void* operator op(size_t count,                                     \
                             std::align_val_t align,                           \
                             First&& first,                                    \
                             Rest&&... rest) /**/                              \
        noexcept(noexcept(T::operator op(count,                                \
                                         align,                                \
                                         std::forward<First>(first),           \
                                         std::forward<Rest>(rest)...)))        \
        requires requires {                                                    \
            {                                                                  \
                T::operator op(count,                                          \
                               align,                                          \
                               std::forward<First>(first),                     \
                               std::forward<Rest>(rest)...)                    \
            };                                                                 \
        }                                                                      \
    {                                                                          \
        return T::operator op(count,                                           \
                              align,                                           \
                              std::forward<First>(first),                      \
                              std::forward<Rest>(rest)...);                    \
    }

#define OVERLOAD_DEALLOCATION(op)                                              \
    static void operator op(void* ptr) /**/                                    \
        noexcept(noexcept(T::operator op(ptr)))                                \
        requires requires { T::operator op(ptr); }                             \
    {                                                                          \
        return T::operator op(ptr);                                            \
    }                                                                          \
                                                                               \
    static void operator op(void* ptr, std::align_val_t align) /**/            \
        noexcept(noexcept(T::operator op(ptr, align)))                         \
        requires requires { T::operator op(ptr, align); }                      \
    {                                                                          \
        return T::operator op(ptr, align);                                     \
    }                                                                          \
                                                                               \
    static void operator op(void* ptr, size_t size) /**/                       \
        noexcept(noexcept(T::operator op(ptr, size)))                          \
        requires requires { T::operator op(ptr, size); }                       \
    {                                                                          \
        return T::operator op(ptr, size);                                      \
    }                                                                          \
                                                                               \
    template <typename First>                                                  \
        requires(!std::is_same_v<std::align_val_t, First> &&                   \
                 !std::is_same_v<size_t, First>)                               \
    static void operator op(void* ptr, First&& first) /**/                     \
        noexcept(noexcept(T::operator op(ptr, std::forward<First>(first))))    \
        requires requires { T::operator op(ptr, std::forward<First>(first)); } \
    {                                                                          \
        return T::operator op(ptr, std::forward<First>(first));                \
    }                                                                          \
                                                                               \
    static void operator delete(void* ptr,                                     \
                                size_t size,                                   \
                                std::align_val_t align) /**/                   \
        noexcept(noexcept(T::operator op(ptr, size, align)))                   \
        requires requires { T::operator op(ptr, size, align); }                \
    {                                                                          \
        return T::operator op(ptr, size, align);                               \
    }                                                                          \
                                                                               \
    template <typename First, typename Second, typename... Rest>               \
        requires(!std::is_same_v<size_t, First> &&                             \
                 !std::is_same_v<std::align_val_t, Second>)                    \
    static void operator op(void* ptr,                                         \
                            First&& first,                                     \
                            Second&& second,                                   \
                            Rest&&... rest) /**/                               \
        noexcept(noexcept(T::operator op(ptr,                                  \
                                         std::forward<First>(first),           \
                                         std::forward<Second>(second),         \
                                         std::forward<Rest>(rest)...)))        \
        requires requires {                                                    \
            T::operator op(ptr,                                                \
                           std::forward<First>(first),                         \
                           std::forward<Second>(second),                       \
                           std::forward<Rest>(rest)...);                       \
        }                                                                      \
    {                                                                          \
        return T::operator op(ptr,                                             \
                              std::forward<First>(first),                      \
                              std::forward<Second>(second),                    \
                              std::forward<Rest>(rest)...);                    \
    }

namespace except::detail {

template <typename T>
    requires utils::concepts::Class<T> && utils::concepts::NonCV<T>
struct AutoObject final {
   private:
    inline void Register() noexcept {}
    inline void Unregister() noexcept {}

    inline void Register() noexcept
        requires std::is_destructible_v<T>
    {
        except::detail::ObjectManager::RecordObject(*this);
    }

    inline void Unregister() noexcept
        requires std::is_destructible_v<T>
    {
        except::detail::ObjectManager::Pop();
    }

   public:
    ////////////////////////////////////////////////////////////////////////////
    ///                           Constructors                               ///
    ////////////////////////////////////////////////////////////////////////////

    // General form

    template <typename... From>
        requires std::is_constructible_v<T, From...>
    explicit(!utils::concepts::ImplicitlyConstructibleFrom<T, From...>)
        // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
        AutoObject(From&&... from)  //
        noexcept(std::is_nothrow_constructible_v<T, From...>)
        : instance_(std::forward<From>(from)...) {
        Register();
    }

    // Initializer list

    template <typename List>
        requires std::is_constructible_v<T, std::initializer_list<List>&>
    explicit(!utils::concepts::ImplicitlyConstructibleFrom<
             T,
             std::initializer_list<List>&>)
        AutoObject(std::initializer_list<List> list)  //
        noexcept(
            std::is_nothrow_constructible_v<T, std::initializer_list<List>>)
        : instance_(list) {
        Register();
    }

    // Initializer list with additional arguments

    template <typename List, typename... Args>
        requires std::
            is_constructible_v<T, std::initializer_list<List>&, Args...>
        explicit(!utils::concepts::ImplicitlyConstructibleFrom<
                 T,
                 std::initializer_list<List>&,
                 Args...>)
            AutoObject(std::initializer_list<List> list, Args... args)  //
        noexcept(std::is_nothrow_constructible_v<T,
                                                 std::initializer_list<List>&,
                                                 Args...>)
        : instance_(list, std::forward<Args>(args)...) {
        Register();
    }

    // Copy

    explicit(!utils::concepts::ImplicitlyConstructibleFrom<T, const T&>)
        AutoObject(const AutoObject& other)  //
        noexcept(std::is_nothrow_copy_constructible_v<T>)
        requires(std::is_copy_constructible_v<T> &&
                 !std::is_constructible_v<T, const AutoObject&>)
        : instance_(other.instance_) {
        Register();
    }

    // Move

    explicit(!utils::concepts::ImplicitlyConstructibleFrom<T, T&&>)
        AutoObject(AutoObject&& other)  //
        noexcept(std::is_nothrow_move_constructible_v<T>)
        requires(std::is_move_constructible_v<T> &&
                 !std::is_constructible_v<T, AutoObject &&>)
        : instance_(std::move(other.instance_)) {
        Register();
    }

    ////////////////////////////////////////////////////////////////////////////
    ///                            Destructor                                ///
    ////////////////////////////////////////////////////////////////////////////

    ~AutoObject() noexcept(std::is_nothrow_destructible_v<T>) {
        Unregister();
    }

    ////////////////////////////////////////////////////////////////////////////
    ///                            Assignments                               ///
    ////////////////////////////////////////////////////////////////////////////

    // NOLINTBEGIN(fuchsia-overloaded-operator)

    // General form

    template <typename U>
    AutoObject& operator=(U&& rhs)  //
        noexcept(noexcept(std::declval<T>().operator=(std::forward<U>(rhs))))
        requires requires(T instance) {
            { instance.operator=(std::forward<U>(rhs)) };
        }
    {
        instance_.operator=(std::forward<U>(rhs));
        return *this;
    }

    template <typename U>
    AutoObject& operator=(U&& rhs) const  //
        noexcept(
            noexcept(std::declval<const T>().operator=(std::forward<U>(rhs))))
        requires requires(const T instance) {
            { instance.operator=(std::forward<U>(rhs)) };
        }
    {
        instance_.operator=(std::forward<U>(rhs));
        return *this;
    }

    // Copy

    AutoObject& operator=(const AutoObject&)  //
        noexcept(std::is_nothrow_copy_assignable_v<T>)
        requires std::is_copy_assignable_v<T>
    = default;

    // Move

    AutoObject& operator=(AutoObject&&)  //
        noexcept(std::is_nothrow_move_assignable_v<T>)
        requires std::is_move_assignable_v<T>
    = default;

    ////////////////////////////////////////////////////////////////////////////
    ///                            Arithmetics                               ///
    ////////////////////////////////////////////////////////////////////////////

    OVERLOAD_UNARY_OPERATOR(+)
    OVERLOAD_UNARY_OPERATOR(-)

    OVERLOAD_BINARY_OPERATOR(+)
    OVERLOAD_BINARY_OPERATOR(-)
    OVERLOAD_BINARY_OPERATOR(*)
    OVERLOAD_BINARY_OPERATOR(/)
    OVERLOAD_BINARY_OPERATOR(%)

    OVERLOAD_BINARY_OPERATOR(+=)
    OVERLOAD_BINARY_OPERATOR(-=)
    OVERLOAD_BINARY_OPERATOR(*=)
    OVERLOAD_BINARY_OPERATOR(/=)
    OVERLOAD_BINARY_OPERATOR(%=)

    ////////////////////////////////////////////////////////////////////////////
    ///                              Bitwise                                 ///
    ////////////////////////////////////////////////////////////////////////////

    OVERLOAD_UNARY_OPERATOR(~)

    OVERLOAD_BINARY_OPERATOR(&)
    OVERLOAD_BINARY_OPERATOR(|)
    OVERLOAD_BINARY_OPERATOR(^)
    OVERLOAD_BINARY_OPERATOR(<<)
    OVERLOAD_BINARY_OPERATOR(>>)

    OVERLOAD_BINARY_OPERATOR(&=)
    OVERLOAD_BINARY_OPERATOR(|=)
    OVERLOAD_BINARY_OPERATOR(^=)
    OVERLOAD_BINARY_OPERATOR(<<=)
    OVERLOAD_BINARY_OPERATOR(>>=)

    ////////////////////////////////////////////////////////////////////////////
    ///                              Logical                                 ///
    ////////////////////////////////////////////////////////////////////////////

    OVERLOAD_UNARY_OPERATOR(!)

    OVERLOAD_BINARY_OPERATOR(&&)
    OVERLOAD_BINARY_OPERATOR(||)

    ////////////////////////////////////////////////////////////////////////////
    ///                            Comparisons                               ///
    ////////////////////////////////////////////////////////////////////////////

    OVERLOAD_BINARY_OPERATOR(==)
    OVERLOAD_BINARY_OPERATOR(!=)
    OVERLOAD_BINARY_OPERATOR(<)
    OVERLOAD_BINARY_OPERATOR(>)
    OVERLOAD_BINARY_OPERATOR(<=)
    OVERLOAD_BINARY_OPERATOR(>=)
    OVERLOAD_BINARY_OPERATOR(<=>)

    ////////////////////////////////////////////////////////////////////////////
    ///                      Increment and decrement                         ///
    ////////////////////////////////////////////////////////////////////////////

    // prefix
    OVERLOAD_UNARY_OPERATOR(++)
    OVERLOAD_UNARY_OPERATOR(--)

    // postfix
    decltype(auto) operator++(int)  // NOLINT(cert-dcl21-cpp)
        noexcept(noexcept(instance_.operator++(0)))
        requires requires(T instance) {
            { instance.operator++(0) };
        }
    {
        return instance_.operator++(0);
    }

    decltype(auto) operator++(int) const  // NOLINT(cert-dcl21-cpp)
        noexcept(noexcept(instance_.operator++(0)))
        requires requires(const T instance) {
            { instance.operator++(0) };
        }
    {
        return instance_.operator++(0);
    }

    decltype(auto) operator--(int)  // NOLINT(cert-dcl21-cpp)
        noexcept(noexcept(instance_.operator--(0)))
        requires requires(T instance) {
            { instance.operator--(0) };
        }
    {
        return instance_.operator--(0);
    }

    decltype(auto) operator--(int) const  // NOLINT(cert-dcl21-cpp)
        noexcept(noexcept(instance_.operator--(0)))
        requires requires(const T instance) {
            { instance.operator--(0) };
        }
    {
        return instance_.operator--(0);
    }

    ////////////////////////////////////////////////////////////////////////////
    ///                               Comma                                  ///
    ////////////////////////////////////////////////////////////////////////////

#define COMMA ,
    OVERLOAD_BINARY_OPERATOR(COMMA)
#undef COMMA

    ////////////////////////////////////////////////////////////////////////////
    ///                          Multi-argument                              ///
    ////////////////////////////////////////////////////////////////////////////

    OVERLOAD_MULTI_ARGUMENT_OPERATOR(())
    OVERLOAD_MULTI_ARGUMENT_OPERATOR([])  // multi-argument since C++23

    ////////////////////////////////////////////////////////////////////////////
    ///                            Allocation                                ///
    ////////////////////////////////////////////////////////////////////////////

    OVERLOAD_ALLOCATION(new)
    OVERLOAD_ALLOCATION(new[])

    ////////////////////////////////////////////////////////////////////////////
    ///                           Deallocation                               ///
    ////////////////////////////////////////////////////////////////////////////

    OVERLOAD_DEALLOCATION(delete)
    OVERLOAD_DEALLOCATION(delete[])

    // Since C++20

    void operator delete(AutoObject* ptr, std::destroying_delete_t destroy)  //
        noexcept(
            noexcept(std::declval<T>().operator delete(std::declval<T*>())))
        requires requires(T instance, T* pointer) {
            instance.operator delete(pointer);
        }
    {
        return ptr->instance_.operator delete(&ptr->instance_, destroy);
    }

    void operator delete(AutoObject* ptr,
                         std::destroying_delete_t destroy,
                         std::align_val_t align)  //
        noexcept(noexcept(std::declval<T>().operator delete(std::declval<T*>(),
                                                            align)))
        requires requires(T instance, T* pointer) {
            instance.operator delete(pointer, align);
        }
    {
        return ptr->instance_.operator delete(&ptr->instance_, destroy, align);
    }

    void operator delete(AutoObject* ptr,
                         std::destroying_delete_t destroy,
                         size_t size)  //
        noexcept(noexcept(std::declval<T>().operator delete(std::declval<T*>(),
                                                            size)))
        requires requires(T instance, T* pointer) {
            instance.operator delete(pointer, size);
        }
    {
        return ptr->instance_.operator delete(&ptr->instance_, destroy, size);
    }

    void operator delete(AutoObject* ptr,
                         std::destroying_delete_t destroy,
                         size_t size,
                         std::align_val_t align)  //
        noexcept(noexcept(
            std::declval<T>().operator delete(std::declval<T*>(), size, align)))
        requires requires(T instance, T* pointer) {
            instance.operator delete(pointer, size, align);
        }
    {
        return ptr->instance_.operator delete(&ptr->instance_,
                                              destroy,
                                              size,
                                              align);
    }

    ////////////////////////////////////////////////////////////////////////////
    ///                           Member access                              ///
    ////////////////////////////////////////////////////////////////////////////

    OVERLOAD_UNARY_OPERATOR(*)
    OVERLOAD_UNARY_OPERATOR(&)  // NOLINT(google-runtime-operator)

    OVERLOAD_BINARY_OPERATOR(->*)

    // operator-> gets chained, so we can simply return T*

    T* operator->() noexcept {
        return &instance_;
    }

    const T* operator->() const noexcept {
        return &instance_;
    }

    ////////////////////////////////////////////////////////////////////////////
    ///                             co_await                                 ///
    ////////////////////////////////////////////////////////////////////////////

    OVERLOAD_UNARY_OPERATOR(co_await)

    ////////////////////////////////////////////////////////////////////////////
    ///                            Conversions                               ///
    ////////////////////////////////////////////////////////////////////////////

    // NOLINTBEGIN(google-explicit-constructor,hicpp-explicit-conversions)

    template <typename U>
        requires(std::is_constructible_v<U, T> &&
                 !std::is_same_v<T, std::remove_cvref_t<U>>)
    explicit(!std::is_convertible_v<T, U>) operator U() noexcept(
        std::is_nothrow_convertible_v<T, U> ||
        (!std::is_convertible_v<T, U> &&
         std::is_nothrow_constructible_v<U, T>)) {
        return static_cast<U>(instance_);
    }

    template <typename U>
        requires(std::is_constructible_v<U, const T> &&
                 !std::is_same_v<T, std::remove_cvref_t<U>>)
    explicit(!std::is_convertible_v<const T, U>) operator U() const
        noexcept(std::is_nothrow_convertible_v<const T, U> ||
                 (!std::is_convertible_v<const T, U> &&
                  std::is_nothrow_constructible_v<U, const T>)) {
        return static_cast<U>(instance_);
    }

    // NOLINTEND(google-explicit-constructor,hicpp-explicit-conversions)
    // NOLINTEND(fuchsia-overloaded-operator)

    ////////////////////////////////////////////////////////////////////////////
    ///                           Direct access                              ///
    ////////////////////////////////////////////////////////////////////////////

    // NOLINTBEGIN(fuchsia-overloaded-operator,google-explicit-constructor,hicpp-explicit-conversions)

    operator T&() & {
        return instance_;
    }

    operator const T&() const& {
        return instance_;
    }

    operator T&&() && {
        return std::move(instance_);
    }

    // NOLINTEND(fuchsia-overloaded-operator,google-explicit-constructor,hicpp-explicit-conversions)

    T& Get() & {
        return instance_;
    }

    const T& Get() const& {
        return instance_;
    }

    // This method aims to resolve the ambiguity when the operators:
    //     operator const T&() const &
    //     operator T&&() &&
    // can both be selected
    //
    // Usage:
    //     std::move(object).Move() // this always gives have T&&
    T&& Move() && {
        return std::move(instance_);
    }

   private:
    T instance_;
};

#undef OVERLOAD_UNARY_OPERATOR
#undef OVERLOAD_BINARY_OPERATOR
#undef OVERLOAD_MULTI_ARGUMENT_OPERATOR
#undef OVERLOAD_ALLOCATION
#undef OVERLOAD_DEALLOCATION

template <typename T>
    requires(utils::concepts::Class<T> &&  //
             utils::concepts::NonCV<T> &&  //
             !std::is_final_v<T>)
struct AutoObject<T> final : T {
   private:
    inline void Register() noexcept {}
    inline void Unregister() noexcept {}

    inline void Register() noexcept
        requires std::is_destructible_v<T>
    {
        except::detail::ObjectManager::RecordObject(*this);
    }

    inline void Unregister() noexcept
        requires std::is_destructible_v<T>
    {
        except::detail::ObjectManager::Pop();
    }

   public:
    // general form constructors

    template <typename... From>
        requires std::is_constructible_v<T, From...>
    explicit(!utils::concepts::ImplicitlyConstructibleFrom<T, From...>)
        // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
        AutoObject(From&&... from)  //
        noexcept(std::is_nothrow_constructible_v<T, From...>)
        : T(std::forward<From>(from)...) {
        Register();
    }

    // initializer list constructor

    template <typename List>
        requires std::is_constructible_v<T, std::initializer_list<List>&>
    explicit(!utils::concepts::ImplicitlyConstructibleFrom<
             T,
             std::initializer_list<List>&>)
        AutoObject(std::initializer_list<List> list)  //
        noexcept(
            std::is_nothrow_constructible_v<T, std::initializer_list<List>>)
        : T(list) {
        Register();
    }

    // initializer list with additional arguments constructor

    template <typename List, typename... Args>
        requires std::
            is_constructible_v<T, std::initializer_list<List>&, Args...>
        explicit(!utils::concepts::ImplicitlyConstructibleFrom<
                 T,
                 std::initializer_list<List>&,
                 Args...>)
            AutoObject(std::initializer_list<List> list, Args... args)  //
        noexcept(std::is_nothrow_constructible_v<T,
                                                 std::initializer_list<List>&,
                                                 Args...>)
        : T(list, std::forward<Args>(args)...) {
        Register();
    }

    // copy constructor

    explicit(!utils::concepts::ImplicitlyConstructibleFrom<T, const T&>)
        AutoObject(const AutoObject& other)  //
        noexcept(std::is_nothrow_copy_constructible_v<T>)
        requires(std::is_copy_constructible_v<T> &&
                 !std::is_constructible_v<T, const AutoObject&>)
        : T(static_cast<const T&>(other)) {
        Register();
    }

    // move constructor

    explicit(!utils::concepts::ImplicitlyConstructibleFrom<T, T&&>)
        AutoObject(AutoObject&& other)  //
        noexcept(std::is_nothrow_move_constructible_v<T>)
        requires(std::is_move_constructible_v<T> &&
                 !std::is_constructible_v<T, AutoObject &&>)
        : T(static_cast<T&&>(other)) {
        Register();
    }

    // destructor

    ~AutoObject() noexcept(std::is_nothrow_destructible_v<T>) {
        Unregister();
    }

    // copy assignment

    AutoObject& operator=(const AutoObject&)  //
        noexcept(std::is_nothrow_copy_assignable_v<T>)
        requires std::is_copy_assignable_v<T>
    = default;

    // NOLINTNEXTLINE(fuchsia-overloaded-operator)
    AutoObject& operator=(const T& other)  //
        noexcept(std::is_nothrow_copy_assignable_v<T>)
        requires std::is_copy_assignable_v<T>
    {
        static_cast<T*>(this)->operator=(other);
        return *this;
    };

    // move assignment

    AutoObject& operator=(AutoObject&&)  //
        noexcept(std::is_nothrow_move_assignable_v<T>)
        requires std::is_move_assignable_v<T>
    = default;

    // NOLINTBEGIN(fuchsia-overloaded-operator)

    AutoObject& operator=(T&& other)  //
        noexcept(std::is_nothrow_move_assignable_v<T>)
        requires std::is_move_assignable_v<T>
    {
        static_cast<T*>(this)->operator=(std::move(other));
        return *this;
    };

    T* operator->() noexcept {
        return this;
    }

    const T* operator->() const noexcept {
        return this;
    }

    // NOLINTEND(fuchsia-overloaded-operator)
};

template <typename T>
    requires utils::concepts::Class<T> && utils::concepts::NonCV<T>
AutoObject(T) -> AutoObject<T>;

}  // namespace except::detail

using except::detail::AutoObject;
