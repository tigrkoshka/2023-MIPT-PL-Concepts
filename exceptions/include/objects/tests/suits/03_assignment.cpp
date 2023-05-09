#include <gtest/gtest.h>

#include <type_traits>  // for is_copy_assignable_v, ...
#include <utility>      // for move

#include "objects/auto.hpp"
#include "objects/tests/suits/fixture.hpp"
#include "objects/tests/utils/concepts.hpp"
#include "objects/tests/utils/sample_class.hpp"
#include "objects/tests/utils/stats.hpp"

namespace except::test::objects {

// NOLINTBEGIN(readability-function-cognitive-complexity)

TYPED_TEST(AutoTest, Assignments) {
    using Plain    = TestFixture::Plain;
    using Explicit = TestFixture::Explicit;
    using Noexcept = TestFixture::Noexcept;

    ResetStats();

    AutoObject<Plain> sample_1{kValueS};
    AutoObject<Plain> sample_2{kConstValueS};
    AutoObject<Plain> sample_3;
    Plain sample_4{kValueS};
    Plain sample_5;

    ASSERT_EQ(sample_1->index, kValueS.size());
    ASSERT_EQ(sample_2->index, kConstValueS.size());
    ASSERT_EQ(sample_3->index, 0);
    ASSERT_EQ(sample_4.index, kValueS.size());
    ASSERT_EQ(sample_5.index, 0);

    // AutoObject = AutoObject

    sample_1 = sample_2;
    ASSERT_EQ(sample_2->index, kConstValueS.size());
    ASSERT_EQ(sample_1->index, kConstValueS.size());

    sample_3 = std::move(sample_2);
    ASSERT_EQ(sample_3->index, kConstValueS.size());

    // AutoObject = Plain

    sample_2 = sample_4;
    ASSERT_EQ(sample_4.index, kValueS.size());
    ASSERT_EQ(sample_2->index, kValueS.size());

    sample_1 = std::move(sample_4);
    ASSERT_EQ(sample_1->index, kValueS.size());

    // Plain = AutoObject

    sample_4 = sample_2;
    ASSERT_EQ(sample_2->index, kValueS.size());
    ASSERT_EQ(sample_4.index, kValueS.size());

    if constexpr (std::same_as<Plain, SimplePlain>) {
        sample_5 = std::move(sample_3);
    } else {
        sample_5 = std::move(sample_3).Move();
    }
    ASSERT_EQ(sample_5.index, kConstValueS.size());

    // With const AutoObject (std::move does not do anything with const values)

    const AutoObject<Plain> sample_const{kValueS};
    sample_1 = sample_const;
    sample_4 = sample_const;
    ASSERT_EQ(sample_const->index, kValueS.size());
    ASSERT_EQ(sample_1->index, kValueS.size());
    ASSERT_EQ(sample_4.index, kValueS.size());

    // With const Plain (std::move does not do anything with const values)

    const Plain sample_const_plain{kValueS};
    sample_1 = sample_const_plain;
    ASSERT_EQ(sample_const_plain.index, kValueS.size());
    ASSERT_EQ(sample_1->index, kValueS.size());

    // clang-format off
    const Stats expected{
        .ctor{
             // sample_3, sample_5
            .default_        = 2,
            // sample_1, sample_2, sample_4, sample_const, sample_const_plain
            .single_argument = 5,
        },
        .assign{
            .copy = 6,
            .move = 3,
        },
    };
    // clang-format on
    ASSERT_EQ(Plain::stats, expected);

    // static checks

    static_assert(std::is_copy_assignable_v<AutoObject<Plain>>);
    static_assert(std::is_copy_assignable_v<AutoObject<Explicit>>);
    static_assert(std::is_copy_assignable_v<AutoObject<Noexcept>>);

    static_assert(std::is_assignable_v<AutoObject<Plain>, const Plain&>);
    static_assert(std::is_assignable_v<AutoObject<Explicit>, const Explicit&>);
    static_assert(std::is_assignable_v<AutoObject<Noexcept>, const Noexcept&>);

    static_assert(std::is_move_assignable_v<AutoObject<Plain>>);
    static_assert(std::is_move_assignable_v<AutoObject<Explicit>>);
    static_assert(std::is_move_assignable_v<AutoObject<Noexcept>>);

    static_assert(std::is_assignable_v<AutoObject<Plain>, Plain>);
    static_assert(std::is_assignable_v<AutoObject<Explicit>, Explicit>);
    static_assert(std::is_assignable_v<AutoObject<Noexcept>, Noexcept>);

    // noexcept checks

    static_assert(not std::is_nothrow_copy_assignable_v<AutoObject<Plain>>);
    static_assert(not std::is_nothrow_copy_assignable_v<AutoObject<Explicit>>);
    static_assert(std::is_nothrow_copy_assignable_v<AutoObject<Noexcept>>);

    static_assert(not std::is_nothrow_assignable_v<AutoObject<Plain>,  //
                                                   const Plain&>);
    static_assert(not std::is_nothrow_assignable_v<AutoObject<Explicit>,
                                                   const Explicit&>);
    static_assert(std::is_nothrow_assignable_v<AutoObject<Noexcept>,  //
                                               const Noexcept&>);

    static_assert(not std::is_nothrow_move_assignable_v<AutoObject<Plain>>);
    static_assert(not std::is_nothrow_move_assignable_v<AutoObject<Explicit>>);
    static_assert(std::is_nothrow_move_assignable_v<AutoObject<Noexcept>>);

    static_assert(not std::is_nothrow_assignable_v<AutoObject<Plain>,  //
                                                   Plain>);
    static_assert(not std::is_nothrow_assignable_v<AutoObject<Explicit>,  //
                                                   Explicit>);
    static_assert(std::is_nothrow_assignable_v<AutoObject<Noexcept>,  //
                                               Noexcept>);
}

// NOLINTEND(readability-function-cognitive-complexity)

}  // namespace except::test::objects
