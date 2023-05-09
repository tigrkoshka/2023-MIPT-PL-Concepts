#include <gtest/gtest.h>

#include <compare>      // for three_way_comparable
#include <type_traits>  // for is_constructible_v, ...
#include <utility>      // for declval

#include "objects/auto.hpp"
#include "objects/tests/suits/fixture.hpp"

namespace except::test::objects {

// NOLINTBEGIN(readability-function-cognitive-complexity)

TYPED_TEST(AutoTest, Comparison) {
    using Plain    = TestFixture::Plain;
    using Explicit = TestFixture::Explicit;
    using Noexcept = TestFixture::Noexcept;

    // NOLINTNEXTLINE(misc-const-correctness)
    AutoObject<Plain> sample             = kValueS;
    const AutoObject<Plain> const_sample = kConstValueS;

    ASSERT_EQ(sample, sample);
    ASSERT_EQ(const_sample, const_sample);

    // because the length of "value" is less than that of "const value"
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

    AutoObject<Plain> sample = kValueS;
    ASSERT_EQ(sample->index, kValueS.size());

    const AutoObject post_incremented = sample++;
    ASSERT_EQ(sample->index, kValueS.size() + 1);
    ASSERT_EQ(post_incremented->index, kValueS.size());

    const AutoObject<Plain> pre_incremented = ++sample;
    ASSERT_EQ(sample->index, kValueS.size() + 2);
    ASSERT_EQ(pre_incremented->index, kValueS.size() + 2);

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

    // decrement is not defined for sample classes

    static_assert(not requires(AutoObject<Plain> object) {
        { object-- };
    });
    static_assert(not requires(AutoObject<Plain> object) {
        { --object };
    });
}

TYPED_TEST(AutoTest, Arithmetic) {
    using Plain    = TestFixture::Plain;
    using Noexcept = TestFixture::Noexcept;

    AutoObject<Plain> sample_1 = kValueS;
    AutoObject<Plain> sample_2 = kValueS;
    ASSERT_EQ(sample_1->index, kValueS.size());
    ASSERT_EQ(sample_2->index, kValueS.size());

    const AutoObject<Plain> added = sample_1 + sample_2;
    ASSERT_EQ(added->index, kValueS.size() * 2);

    const AutoObject<Plain> subtracted = sample_1 - sample_2;
    ASSERT_EQ(subtracted->index, 0);

    // do not wrap in AutoObject, because no object is created here,
    // so no destructor must be recorded, to avoid accidentally writing
    // AutoObject<Plain&> the concept for AutoObject template parameter
    // will not allow that (see Template/Concept test above)

    // NOLINTNEXTLINE(misc-const-correctness)
    Plain& added_again = sample_1 += sample_2;
    ASSERT_EQ(added_again.index, 2 * kValueS.size());
    ASSERT_EQ(sample_1->index, 2 * kValueS.size());

    // it is the same object as the one stored in sample_1
    added_again.index = kValueSZ;
    ASSERT_EQ(sample_1->index, kValueSZ);

    Plain& subtracted_again = sample_1 -= sample_2;
    ASSERT_EQ(subtracted_again.index, kValueSZ - kValueS.size());
    ASSERT_EQ(sample_1->index, kValueSZ - kValueS.size());

    // it is the same object as the one stored in sample_1
    subtracted_again.index = kValueSZ;
    ASSERT_EQ(sample_1->index, kValueSZ);

    // noexcept checks

    static_assert(not noexcept(std::declval<AutoObject<Plain>&>()++));
    static_assert(not noexcept(++std::declval<AutoObject<Plain>&>()));
    static_assert(noexcept(std::declval<AutoObject<Noexcept>&>()++));
    static_assert(noexcept(++std::declval<AutoObject<Noexcept>&>()));

    // multiplication and bitwise or are not defined for sample classes

    static_assert(not requires(AutoObject<Plain> object_1,  //
                               AutoObject<Plain> object_2) {
        { object_1 |= object_2 };
    });
    static_assert(not requires(AutoObject<Plain> object_1,  //
                               AutoObject<Plain> object_2) {
        { object_1 *= object_2 };
    });
}

// NOLINTEND(readability-function-cognitive-complexity)

}  // namespace except::test::objects
