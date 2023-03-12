#pragma once

namespace karma::detail::utils::traits {

struct NonCopyableMovable {
    NonCopyableMovable()  = default;
    ~NonCopyableMovable() = default;

    // non-copyable
    NonCopyableMovable(const NonCopyableMovable&)            = delete;
    NonCopyableMovable& operator=(const NonCopyableMovable&) = delete;

    // movable
    NonCopyableMovable(NonCopyableMovable&&)            = default;
    NonCopyableMovable& operator=(NonCopyableMovable&&) = default;
};

}  // namespace karma::detail::utils::traits