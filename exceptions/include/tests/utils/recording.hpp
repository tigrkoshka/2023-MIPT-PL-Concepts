#pragma once

#include <cstddef>  // for destructors
#include <vector>   // for vector

namespace except::test::impl {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
struct Recording {
    Recording();
    // NOLINTNEXTLINE(bugprone-exception-escape)
    ~Recording() noexcept;

    static void Reset();

    static std::vector<size_t> destructors;
    static size_t index;

   private:
    size_t index_;
};

}  // namespace except::test::impl
