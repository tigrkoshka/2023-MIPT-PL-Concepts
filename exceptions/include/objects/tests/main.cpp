#include <gtest/gtest.h>

#include <concepts>
#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>

#include "objects/auto.hpp"

namespace except::test::objects {

using except::detail::AutoObject;

namespace ctor_dtor {

struct Stats {
    struct Constructor {
        // NOLINTNEXTLINE(fuchsia-overloaded-operator)
        bool operator<=>(const Constructor&) const = default;

        friend std::ostream operator<<(std::ostream& out, const Stats& stats) {
            out << "default: " << stats.default_ << std::endl
                << "copy: " << stats.copy << std::endl
        }

        size_t default_{0};
        size_t copy{0};
        size_t move{0};
        size_t single_argument{0};
        size_t multi_argument{0};
    };

    // NOLINTNEXTLINE(fuchsia-overloaded-operator)
    bool operator<=>(const Stats&) const = default;

    Constructor ctor;
    size_t dtor{0};
};

template <bool Explicit, bool Noexcept>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
struct NonTrivial {
    explicit(Explicit) NonTrivial() noexcept(Noexcept) {
        ++stats.ctor.default_;
    }

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    explicit(Explicit) NonTrivial(const std::string& str) noexcept(Noexcept)
        : index(str.size()) {
        ++stats.ctor.single_argument;
    }

    explicit(Explicit)
        NonTrivial(size_t idx, const std::string& str) noexcept(Noexcept)
        : index(idx + str.size()) {
        ++stats.ctor.multi_argument;
    }

    explicit(Explicit) NonTrivial(const NonTrivial& other) noexcept(Noexcept)
        : index(other.index) {
        ++stats.ctor.copy;
    }
    // NOLINTNEXTLINE(performance-noexcept-move-constructor,hicpp-noexcept-move)
    explicit(Explicit) NonTrivial(NonTrivial&& other) noexcept(Noexcept)
        : index(std::exchange(other.index, 0)) {
        ++stats.ctor.move;
    }

    ~NonTrivial() noexcept(Noexcept) {
        ++stats.dtor;
    }

    size_t index{0};
    static Stats stats;
};

template <bool Explicit, bool Noexcept>
Stats NonTrivial<Explicit, Noexcept>::stats;

using Plain    = NonTrivial<false, false>;
using Explicit = NonTrivial<true, false>;
using Noexcept = NonTrivial<false, true>;

void ResetStats() {
    Plain::stats    = Stats{};
    Explicit::stats = Stats{};
    Noexcept::stats = Stats{};
}

template <typename T>
void TestImplicitConstruction(T) {}

template <typename T, typename... Args>
concept From = std::is_constructible_v<T, Args...>;

template <typename T, typename... Args>
concept ImplicitFrom = requires {
    { TestImplicitConstruction<T>({std::declval<Args>()...}) };
};

TEST(Constructors, Default) {
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

    ASSERT_EQ(Plain::stats, Stats{.ctor{.default_ = 3}});
    ASSERT_EQ(Explicit::stats, Stats{.ctor{.default_ = 2}});

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

TEST(Constructors, SingleArgument) {
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

    ASSERT_EQ(Plain::stats, Stats{.ctor{.single_argument = 4}});
    ASSERT_EQ(Explicit::stats, Stats{.ctor{.single_argument = 2}});

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

TEST(Constructors, MultiArgument) {
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

    ASSERT_EQ(Plain::stats, Stats{.ctor{.multi_argument = 4}});
    ASSERT_EQ(Explicit::stats, Stats{.ctor{.multi_argument = 2}});

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

TEST(Constructors, Copy) {
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

    ASSERT_EQ(Plain::stats, (Stats{.ctor{.default_ = 1, .copy = 2}}));
    ASSERT_EQ(Explicit::stats, (Stats{.ctor{.default_ = 1, .copy = 1}}));

    // static checks

    // same as From<AutoObject<Plain>, const AutoObject<Plain>&>
    static_assert(std::is_copy_constructible_v<AutoObject<Plain>>);
    static_assert(ImplicitFrom<AutoObject<Plain>, const AutoObject<Plain>&>);

    // same as From<AutoObject<Explicit>, const AutoObject<Explicit>&>
    static_assert(std::is_copy_constructible_v<AutoObject<Explicit>>);
    static_assert(
        not ImplicitFrom<AutoObject<Explicit>, const AutoObject<Explicit>&>);

    // noexcept checks

    static_assert(not std::is_nothrow_copy_constructible_v<AutoObject<Plain>>);
    static_assert(std::is_nothrow_copy_constructible_v<AutoObject<Noexcept>>);
}

TEST(Constructors, Move) {
    AutoObject<Plain> plain_1;
    AutoObject<Explicit> explicit_1;

    // explicit call

    AutoObject<Plain> plain_2{std::move(plain_1)};
    const AutoObject<Explicit> explicit_2{std::move(explicit_1)};

    // implicit call

    const AutoObject<Plain> plain_3 = std::move(plain_2);
    // const AutoObject<Explicit> fail_1 = std::move(explicit_2);

    // stats checks

    ASSERT_EQ(Plain::stats, (Stats{.ctor{.default_ = 1, .move = 2}}));
    ASSERT_EQ(Explicit::stats, (Stats{.ctor{.default_ = 1, .move = 1}}));

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

}  // namespace ctor_dtor

}  // namespace except::test::objects
