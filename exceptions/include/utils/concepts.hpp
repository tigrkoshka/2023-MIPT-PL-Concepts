#pragma once

#include <concepts> // for constructible_from
#include <utility>  // for declval

namespace except::detail::utils::concepts {

template <typename T>
void Test(T) {}

template <typename T, typename... From>
concept IsImplicitlyConstructible = requires {
    { Test<T>({std::declval<From>()...}) };
};

template <typename T, typename... From>
concept IsOnlyExplicitlyConstructible = std::constructible_from<T, From...> &&
                                        !IsImplicitlyConstructible<T, From...>;

}  // namespace except::detail::utils::concepts
