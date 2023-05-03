#pragma once

#include <concepts>     // for constructible_from
#include <type_traits>  // for is_class_v
#include <utility>      // for declval, in_place_type_t

namespace except::detail::utils::concepts {

namespace detail {

template <typename, template <typename...> typename>
struct IsInstanceOf : public std::false_type {};

template <template <typename...> typename U, typename... Ts>
struct IsInstanceOf<U<Ts...>, U> : public std::true_type {};

}  // namespace detail

template <typename T, template <typename...> typename U>
concept InstanceOf = detail::IsInstanceOf<std::remove_cvref_t<T>, U>::value;

template <typename T>
concept InplaceType = InstanceOf<T, std::in_place_type_t>;

template <typename T>
concept Class = requires { requires std::is_class_v<T>; };

template <typename T>
concept NonCVRef = std::same_as<T, std::remove_cvref_t<T>>;

template <typename T>
concept Decayed = std::same_as<T, std::decay_t<T>>;

namespace detail {

template <typename T>
void TestImplicitConstruction(T) {}

}  // namespace detail

template <typename T, typename... From>
concept ImplicitlyConstructible = requires {
    { detail::TestImplicitConstruction<T>({std::declval<From>()...}) };
};

template <typename T, typename... From>
concept OnlyExplicitlyConstructible =
    std::constructible_from<T, From...> && !ImplicitlyConstructible<T, From...>;

}  // namespace except::detail::utils::concepts
