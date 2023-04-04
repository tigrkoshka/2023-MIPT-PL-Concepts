#pragma once

#include <concepts>    // for integral, floating_point, convertible_to
#include <cstddef>     // for size_t
#include <functional>  // for hash

namespace karma::detail::utils::concepts {

template <typename T>
concept IntOrFloat = std::integral<T> || std::floating_point<T>;

template <typename T>
concept Hashable = requires(T val) {
    { std::hash<T>{}(val) } -> std::convertible_to<std::size_t>;
};

}  // namespace karma::detail::utils::concepts
