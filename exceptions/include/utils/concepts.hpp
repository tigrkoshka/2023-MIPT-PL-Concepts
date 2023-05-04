#pragma once

#include <concepts>     // for constructible_from
#include <type_traits>  // for is_class_v, remove_cvref_t, decay_t
#include <utility>      // for declval

namespace except::detail::utils::concepts {

template <typename T>
concept Class = std::is_class_v<T>;

template <typename T>
concept NonCVRef = std::same_as<T, std::remove_cvref_t<T>>;

template <typename T>
concept Decayed = std::same_as<T, std::decay_t<T>>;

namespace detail {

template <typename T>
void TestImplicitConstruction(T) {}

}  // namespace detail

template <typename T, typename... From>
concept ImplicitlyConstructibleFrom = requires {
    { detail::TestImplicitConstruction<T>({std::declval<From>()...}) };
};

template <typename T, typename U = std::decay_t<T>>
concept Throwable = std::copy_constructible<U> && std::is_destructible_v<U>;

template <typename T>
concept DecayedThrowable = Throwable<T> && Decayed<T>;

template <typename T, typename UnCVRef = std::remove_cvref_t<T>>
concept Catchable = (std::is_reference_v<T> || std::copy_constructible<T>)&&  //
                    utils::concepts::DecayedThrowable<UnCVRef>;

}  // namespace except::detail::utils::concepts
