#pragma once

#include <type_traits>  // for is_class_v, remove_cvref_t, decay_t
#include <utility>      // for declval

namespace except::detail::utils::concepts {

template <typename T>
concept Class = std::is_class_v<T>;

template <typename T>
concept NonCV = std::is_same_v<T, std::remove_cv_t<T>>;

template <typename T>
concept NonCVRef = std::is_same_v<T, std::remove_cvref_t<T>>;

template <typename T>
concept Decayed = std::is_same_v<T, std::decay_t<T>>;

namespace detail {

template <typename T>
void TestImplicitConstruction(T) {}

}  // namespace detail

template <typename T, typename... From>
concept ImplicitlyConstructibleFrom = requires {
    { detail::TestImplicitConstruction<T>({std::declval<From>()...}) };
};

template <typename T, typename U = std::decay_t<T>>
concept Throwable = std::is_copy_constructible_v<U> &&  //
                    std::is_destructible_v<U>;

template <typename T>
concept DecayedThrowable = Throwable<T> && Decayed<T>;

template <typename T, typename Decayed = std::decay_t<T>>
concept Catchable = !std::is_abstract_v<Decayed> &&    //
                    !std::is_rvalue_reference_v<T> &&  //
                    Throwable<Decayed> &&              //
                    (std::is_reference_v<T> || std::is_copy_constructible_v<T>);

}  // namespace except::detail::utils::concepts
