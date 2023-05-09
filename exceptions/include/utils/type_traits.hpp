#include <type_traits>

namespace except::detail::utils::type_traits {

template <typename T>
using CatchType =
    std::conditional_t<std::is_array_v<T> || std::is_function_v<T>,
                       std::decay_t<T>,
                       T>;

} // namespace except::detail::utils::type_traits
