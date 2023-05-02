#pragma once

#include <concepts>     // for constructible_from
#include <type_traits>  // for is_class_v
#include <utility>      // for declval

namespace except::detail::utils::concepts {

template <typename T>
void Test(T) {}

template <typename T>
concept IsClass = requires { requires std::is_class_v<T>; };

template <typename T>
concept IsPure = std::same_as<T, std::decay_t<T>>;

template <typename T, typename... From>
concept IsImplicitlyConstructible = requires {
    { Test<T>({std::declval<From>()...}) };
};

template <typename T, typename... From>
concept IsOnlyExplicitlyConstructible = std::constructible_from<T, From...> &&
                                        !IsImplicitlyConstructible<T, From...>;

}  // namespace except::detail::utils::concepts
