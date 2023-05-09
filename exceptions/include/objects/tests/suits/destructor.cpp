#include <gtest/gtest.h>

#include <type_traits>  // for is_destructible_v, ...

#include "objects/auto.hpp"
#include "objects/tests/suits/fixture.hpp"
#include "objects/tests/utils/sample_class.hpp"
#include "objects/tests/utils/stats.hpp"

namespace except::test::objects {

// NOLINTBEGIN(readability-function-cognitive-complexity)

TYPED_TEST(AutoTest, Destructor) {
    using Plain    = TestFixture::Plain;
    using Explicit = TestFixture::Explicit;
    using Noexcept = TestFixture::Noexcept;

    ResetStats();

    {
        const AutoObject<Plain> sample_1;
        const AutoObject<Explicit> sample_2;
        const AutoObject<Noexcept> sample_3;
    }

    // stats checks

    const Stats expected{
        .ctor{.default_ = 1},
        .dtor = 1,
    };
    ASSERT_EQ(Plain::stats, expected);
    ASSERT_EQ(Explicit::stats, expected);
    ASSERT_EQ(Noexcept::stats, expected);

    // static checks

    static_assert(std::is_destructible_v<AutoObject<Plain>>);
    static_assert(std::is_destructible_v<AutoObject<Explicit>>);
    static_assert(std::is_destructible_v<AutoObject<Noexcept>>);

    // noexcept checks

    static_assert(not std::is_nothrow_destructible_v<AutoObject<Plain>>);
    static_assert(not std::is_nothrow_destructible_v<AutoObject<Explicit>>);
    static_assert(std::is_nothrow_destructible_v<AutoObject<Noexcept>>);
}

// NOLINTEND(readability-function-cognitive-complexity)

}  // namespace except::test::objects
