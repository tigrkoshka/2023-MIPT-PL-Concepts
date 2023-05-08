#include <gtest/gtest.h>

#include <compare>      // for three_way_comparable
#include <string>       // for string
#include <type_traits>  // for is_constructible_v, ...
#include <utility>      // for declval

#include "objects/auto.hpp"
#include "objects/tests/suits/fixture.hpp"
#include "objects/tests/utils/sample_class.hpp"
#include "objects/tests/utils/stats.hpp"

namespace except::test::objects {

TYPED_TEST(AutoTest, Comparison) {
    using Plain    = TestFixture::Plain;
    using Explicit = TestFixture::Explicit;
    using Noexcept = TestFixture::Noexcept;

    // NOLINTNEXTLINE(misc-const-correctness)
    AutoObject<Plain> sample             = "plain";
    const AutoObject<Plain> const_sample = "const plain";

    ASSERT_EQ(sample, sample);
    ASSERT_EQ(const_sample, const_sample);

    // because the length of "plain" is less than that of "const plain"
    ASSERT_LT(sample, const_sample);
    ASSERT_GT(const_sample, sample);

    // static checks

    static_assert(std::three_way_comparable<AutoObject<Plain>>);
    static_assert(std::three_way_comparable<AutoObject<Explicit>>);
    static_assert(std::three_way_comparable<AutoObject<Noexcept>>);

    // noexcept checks

    static_assert(
        not noexcept(std::declval<Plain>() <=> std::declval<Plain>()));
    static_assert(
        not noexcept(std::declval<Explicit>() <=> std::declval<Explicit>()));
    static_assert(
        noexcept(std::declval<Noexcept>() <=> std::declval<Noexcept>()));
}

TYPED_TEST(AutoTest, Increment) {
    using Plain    = TestFixture::Plain;
    using Noexcept = TestFixture::Noexcept;

    const std::string value = "plain";

    AutoObject<Plain> sample = value;
    ASSERT_EQ(sample->index, value.size());

    const AutoObject post_incremented = sample++;
    ASSERT_EQ(sample->index, value.size() + 1);
    ASSERT_EQ(post_incremented->index, value.size());

    const AutoObject<Plain> pre_incremented = ++sample;
    ASSERT_EQ(sample->index, value.size() + 2);
    ASSERT_EQ(pre_incremented->index, value.size() + 2);

    // do not wrap in AutoObject, because no object is created here,
    // so no destructor must be recorded, to avoid accidentally writing
    // AutoObject<Plain&> the concept for AutoObject template parameter
    // will not allow that (see Template/Concept test above)

    // NOLINTNEXTLINE(misc-const-correctness)
    [[maybe_unused]] Plain& pre_incremented_again = ++sample;

    // noexcept checks

    static_assert(not noexcept(std::declval<AutoObject<Plain>&>()++));
    static_assert(not noexcept(++std::declval<AutoObject<Plain>&>()));
    static_assert(noexcept(std::declval<AutoObject<Noexcept>&>()++));
    static_assert(noexcept(++std::declval<AutoObject<Noexcept>&>()));
}

}  // namespace except::test::objects
