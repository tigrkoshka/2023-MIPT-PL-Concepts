#pragma once

#include <type_traits>
#include <utility>

#include "objects/auto.hpp"

namespace except::test::objects {

template <typename T>
void TestImplicitConstruction(T);  // not defined

template <typename T, typename... Args>
concept From = std::is_constructible_v<T, Args...>;

template <typename T, typename... Args>
concept ImplicitFrom = requires {
    { TestImplicitConstruction<T>({std::declval<Args>()...}) };
};

template <typename T>
concept CanAutoObject = requires { typename AutoObject<T>; };

template <typename T>
concept CanIndex = requires {
    { std::declval<T>().Index() };
};

template <typename T>
concept CanIncrementIndex = requires {
    { std::declval<T>().IncrementIndex() };
};

}  // namespace except::test::objects
