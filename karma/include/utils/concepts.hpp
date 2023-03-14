#pragma once

#include <concepts>    // for integral, floating_point, convertible_to
#include <cstddef>     // for size_t
#include <functional>  // for hash
#include <istream>     // for istream

namespace karma::detail::utils::concepts {

template <typename T>
concept IntOrFloat = std::integral<T> || std::floating_point<T>;

// TODO: turn formatting on when clang-format@16 (in llvm@16) is released
// clang-format does not yet properly support concepts definition
// clang-format off

template <typename T>
concept Hashable = requires(T val) {
    { std::hash<T>{}(val) } -> std::convertible_to<std::size_t>;
};

// clang-format on

}  // namespace karma::detail::utils::concepts