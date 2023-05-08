#include <gtest/gtest.h>

#include <cstddef>      // for size_t
#include <memory>       // for allocator
#include <string>       // for string
#include <type_traits>  // for is_default_constructible_v, ...
#include <utility>      // for move
#include <vector>       // for vector

#include "objects/auto.hpp"
#include "objects/tests/suits/fixture.hpp"
#include "objects/tests/utils/concepts.hpp"
#include "objects/tests/utils/sample_class.hpp"
#include "objects/tests/utils/stats.hpp"

namespace except::test::objects {

TYPED_TEST(AutoTest, DefaultConstructor) {
    using Plain    = TestFixture::Plain;
    using Explicit = TestFixture::Explicit;
    using Noexcept = TestFixture::Noexcept;

    ResetStats();

    // explicit call

    const AutoObject<Plain> sample_1;
    const AutoObject<Plain> sample_2{};
    const AutoObject<Explicit> sample_3;
    const AutoObject<Explicit> sample_4{};

    // implicit call

    const AutoObject<Plain> sample_5 = {};
    // const AutoObject<Explicit> fail_1 = {};

    // stats checks

    ASSERT_EQ(Plain::stats, (Stats{.ctor{.default_ = 3}}));
    ASSERT_EQ(Explicit::stats, (Stats{.ctor{.default_ = 2}}));

    // static checks

    static_assert(From<AutoObject<Plain>>);
    static_assert(ImplicitFrom<AutoObject<Plain>>);

    static_assert(From<AutoObject<Explicit>>);
    static_assert(not ImplicitFrom<AutoObject<Explicit>>);

    // noexcept checks

    static_assert(
        not std::is_nothrow_default_constructible_v<AutoObject<Plain>>);
    static_assert(
        std::is_nothrow_default_constructible_v<AutoObject<Noexcept>>);
}

TYPED_TEST(AutoTest, SingleArgumentConstructors) {
    using Plain    = TestFixture::Plain;
    using Explicit = TestFixture::Explicit;
    using Noexcept = TestFixture::Noexcept;

    ResetStats();

    // explicit call

    const AutoObject<Plain> sample_1{std::string("test")};
    const AutoObject<Plain> sample_2{"test"};
    const AutoObject<Explicit> sample_3{std::string("test")};
    const AutoObject<Explicit> sample_4{"test"};

    // implicit call

    const AutoObject<Plain> sample_5 = {std::string("test")};
    const AutoObject<Plain> sample_6 = {"test"};
    // const AutoObject<Explicit> fail_1 = {std::string("test")};
    // const AutoObject<Explicit> fail_2 = {"test"};

    // stats checks

    ASSERT_EQ(Plain::stats, (Stats{.ctor{.single_argument = 4}}));
    ASSERT_EQ(Explicit::stats, (Stats{.ctor{.single_argument = 2}}));

    // static checks

    static_assert(From<AutoObject<Plain>, std::string>);
    static_assert(From<AutoObject<Plain>, const char*>);
    static_assert(ImplicitFrom<AutoObject<Plain>, std::string>);
    static_assert(ImplicitFrom<AutoObject<Plain>, const char*>);

    static_assert(From<AutoObject<Explicit>, std::string>);
    static_assert(From<AutoObject<Explicit>, const char*>);
    static_assert(not ImplicitFrom<AutoObject<Explicit>, std::string>);
    static_assert(not ImplicitFrom<AutoObject<Explicit>, const char*>);

    // check noexcept

    static_assert(
        not std::is_nothrow_constructible_v<AutoObject<Plain>, std::string>);
    static_assert(
        not std::is_nothrow_constructible_v<AutoObject<Plain>, const char*>);

    static_assert(
        std::is_nothrow_constructible_v<AutoObject<Noexcept>, std::string>);
    // because const char* -> std::string is not noexcept (see below)
    static_assert(
        not std::is_nothrow_constructible_v<AutoObject<Noexcept>, const char*>);
    static_assert(
        not std::is_nothrow_constructible_v<std::string, const char*>);
}

TYPED_TEST(AutoTest, MultiArgumentConstructors) {
    using Plain    = TestFixture::Plain;
    using Explicit = TestFixture::Explicit;
    using Noexcept = TestFixture::Noexcept;

    ResetStats();

    // explicit call

    const AutoObject<Plain> sample_1{239ull, std::string("test")};
    const AutoObject<Plain> sample_2{239ull, "test"};
    const AutoObject<Explicit> sample_3{239ull, std::string("test")};
    const AutoObject<Explicit> sample_4{239ull, "test"};

    // implicit call

    const AutoObject<Plain> sample_5 = {239ull, std::string("test")};
    const AutoObject<Plain> sample_6 = {239ull, "test"};
    // const AutoObject<Explicit> fail_1 = {239ull, std::string("test")};
    // const AutoObject<Explicit> fail_2 = {239ull, "test"};

    // stats checks

    ASSERT_EQ(Plain::stats, (Stats{.ctor{.multi_argument = 4}}));
    ASSERT_EQ(Explicit::stats, (Stats{.ctor{.multi_argument = 2}}));

    // static checks

    static_assert(From<AutoObject<Plain>, size_t, std::string>);
    static_assert(From<AutoObject<Plain>, size_t, const char*>);
    static_assert(ImplicitFrom<AutoObject<Plain>, size_t, std::string>);
    static_assert(ImplicitFrom<AutoObject<Plain>, size_t, const char*>);

    static_assert(From<AutoObject<Explicit>, size_t, std::string>);
    static_assert(From<AutoObject<Explicit>, size_t, const char*>);
    static_assert(not ImplicitFrom<AutoObject<Explicit>, size_t, std::string>);
    static_assert(not ImplicitFrom<AutoObject<Explicit>, size_t, const char*>);

    // check noexcept

    static_assert(not std::is_nothrow_constructible_v<AutoObject<Plain>,
                                                      size_t,
                                                      std::string>);
    static_assert(not std::is_nothrow_constructible_v<AutoObject<Plain>,
                                                      size_t,
                                                      const char*>);

    static_assert(std::is_nothrow_constructible_v<AutoObject<Noexcept>,
                                                  size_t,
                                                  std::string>);
    // because const char* -> std::string is not noexcept (see below)
    static_assert(not std::is_nothrow_constructible_v<AutoObject<Noexcept>,
                                                      size_t,
                                                      const char*>);
    static_assert(
        not std::is_nothrow_constructible_v<std::string, const char*>);
}

TYPED_TEST(AutoTest, InitListConstructors) {
    const AutoObject<std::vector<int>> sample_1 = {1, 2, 3};
    const AutoObject<std::vector<int>> sample_2{1, 2, 3};

    const AutoObject<std::vector<int>> sample_3 = {
        {1, 2, 3},
        std::allocator<int>()
    };

    const AutoObject<std::vector<int>> sample_4{
        {1, 2, 3},
        std::allocator<int>()
    };
}

TYPED_TEST(AutoTest, CopyConstructors) {
    using Plain    = TestFixture::Plain;
    using Explicit = TestFixture::Explicit;
    using Noexcept = TestFixture::Noexcept;

    ResetStats();

    const AutoObject<Plain> plain_1;
    const AutoObject<Explicit> explicit_1;

    // explicit call

    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    const AutoObject<Plain> plain_2{plain_1};
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    const AutoObject<Explicit> explicit_2{explicit_1};

    // implicit call

    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    const AutoObject<Plain> plain_3 = plain_1;
    // const AutoObject<Explicit> fail_1 = explicit_1;

    // stats checks

    // clang-format off
    ASSERT_EQ(Plain::stats, (Stats{.ctor{.default_ = 1, .copy = 2}}));
    ASSERT_EQ(Explicit::stats, (Stats{.ctor{.default_ = 1, .copy = 1}}));
    // clang-format on

    // static checks

    // same as From<AutoObject<Plain>, const AutoObject<Plain>&>
    static_assert(std::is_copy_constructible_v<AutoObject<Plain>>);
    static_assert(ImplicitFrom<AutoObject<Plain>, const AutoObject<Plain>&>);

    // same as From<AutoObject<Explicit>, const
    // AutoObject<Explicit>&>
    static_assert(std::is_copy_constructible_v<AutoObject<Explicit>>);
    static_assert(
        not ImplicitFrom<AutoObject<Explicit>, const AutoObject<Explicit>&>);

    // noexcept checks

    static_assert(not std::is_nothrow_copy_constructible_v<AutoObject<Plain>>);
    static_assert(std::is_nothrow_copy_constructible_v<AutoObject<Noexcept>>);
}

TYPED_TEST(AutoTest, MoveConstructors) {
    using Plain    = TestFixture::Plain;
    using Explicit = TestFixture::Explicit;
    using Noexcept = TestFixture::Noexcept;

    ResetStats();

    AutoObject<Plain> plain_1;
    AutoObject<Explicit> explicit_1;

    // explicit call

    AutoObject<Plain> plain_2{std::move(plain_1)};
    const AutoObject<Explicit> explicit_2{std::move(explicit_1)};

    // implicit call

    const AutoObject<Plain> plain_3 = std::move(plain_2);
    // const AutoObject<Explicit> fail_1 = std::move(explicit_2);

    // stats checks

    // clang-format off
    ASSERT_EQ(Plain::stats, (Stats{.ctor{.default_ = 1, .move = 2}}));
    ASSERT_EQ(Explicit::stats, (Stats{.ctor{.default_ = 1, .move = 1}}));
    // clang-format on

    // static checks

    // same as From<AutoObject<Explicit>, AutoObject<Explicit>&&>
    static_assert(std::is_move_constructible_v<AutoObject<Plain>>);
    static_assert(ImplicitFrom<AutoObject<Plain>, AutoObject<Plain>&&>);

    // same as From<AutoObject<Explicit>, AutoObject<Explicit>&&>
    static_assert(std::is_move_constructible_v<AutoObject<Explicit>>);
    static_assert(
        not ImplicitFrom<AutoObject<Explicit>, AutoObject<Explicit>&&>);

    // noexcept checks

    static_assert(not std::is_nothrow_move_constructible_v<AutoObject<Plain>>);
    static_assert(std::is_nothrow_move_constructible_v<AutoObject<Noexcept>>);
}

}  // namespace except::test::objects
