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

struct Static {
    Static()  = delete;
    ~Static() = delete;

    // there is no actual need to define move and copy assignment operators and
    // constructors since a Static object cannot be created, but we do that
    // for consistency

    Static(const Static&) = delete;
    Static& operator=(const Static&) = delete;

    Static(Static&&) = delete;
    Static& operator=(Static&&) = delete;
};

}  // namespace karma::detail::utils::traits