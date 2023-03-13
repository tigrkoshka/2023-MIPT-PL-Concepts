#pragma once

#include <algorithm>  // for copy
#include <concepts>   // for same_as
#include <cstddef>    // for size_t
#include <vector>     // for vector

namespace karma::detail::utils::vector {

template <typename T, typename... Args>
    requires(std::same_as<Args, std::vector<T>> && ...)
void Append(std::vector<T>& dst, const Args&... srcs) {
    (dst.insert(dst.end(), srcs.begin(), srcs.end()), ...);
}

template <typename T, typename... Args>
    requires(std::same_as<Args, std::vector<T>> && ...)
void CopyToBegin(std::vector<T>& dst, const Args&... srcs) {
    typename std::vector<T>::iterator::difference_type start = 0;

    ((std::copy(srcs.begin(), srcs.end(), dst.begin() + start),
      start += srcs.size()),
     ...);
}

}  // namespace karma::detail::utils::vector