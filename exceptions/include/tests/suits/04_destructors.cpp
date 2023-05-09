#include <gtest/gtest.h>

#include <cstddef>  // for size_t
#include <vector>   // for vector

//
#include <except>

#include "tests/utils/recording.hpp"

namespace except::test::impl {

// NOLINTBEGIN(readability-function-cognitive-complexity)

TEST(Destructors, NotCalledSimple) {
    Recording::Reset();

    const AutoObject<Recording> object;

    TRY {
        Throw(Exception{});
    }
    CATCH(Exception) {
        // OK
    }

    ASSERT_EQ(Recording::destructors, std::vector<size_t>{});
}

TEST(Destructors, CalledSimple) {
    Recording::Reset();

    TRY {
        const AutoObject<Recording> object;
        Throw(Exception{});
    }
    CATCH(Exception) {
        // OK
    }

    const std::vector<size_t> expected{0};
    ASSERT_EQ(Recording::destructors, expected);
}

TEST(Destructors, CallOrderSimple) {
    Recording::Reset();

    TRY {
        const AutoObject<Recording> object_0;
        const AutoObject<Recording> object_1;
        Throw(Exception{});
    }
    CATCH(Exception) {
        // OK
    }

    const std::vector<size_t> expected{1, 0};
    ASSERT_EQ(Recording::destructors, expected);
}

TEST(Destructors, CallOrder) {
    Recording::Reset();

    std::vector<size_t> expected;

    TRY {
        const AutoObject<Recording> object_0;
        const AutoObject<Recording> object_1;

        TRY {
            TRY {
                TRY {
                    const AutoObject<Recording> object_2;
                    const AutoObject<Recording> object_3;
                    Throw(Exception{});
                }
                CATCH(Exception) {
                    expected.push_back(3);
                    expected.push_back(2);
                    ASSERT_EQ(Recording::destructors, expected);

                    const AutoObject<Recording> object_4;
                    Throw();
                }
            }
            CATCH(Exception) {
                expected.push_back(4);
                ASSERT_EQ(Recording::destructors, expected);
                // OK
            }

            Throw(Exception{});
        }
        CATCH(Exception) {
            // check that nothing got destroyed
            ASSERT_EQ(Recording::destructors, expected);

            TRY {
                const AutoObject<Recording> object_5;
            }
            CATCH() {
                // should not enter
                ASSERT_TRUE(false);
            }

            // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
            expected.push_back(5);  // NOLINT(readability-magic-numbers)
            ASSERT_EQ(Recording::destructors, expected);
        }

        Throw(Exception{});
    }
    CATCH(Exception) {
        expected.push_back(1);
        expected.push_back(0);
        ASSERT_EQ(Recording::destructors, expected);
    }
}

void Baz() {
    Throw(RuntimeError{});
}

// NOLINTBEGIN(performance-unnecessary-value-param)

void Bar([[maybe_unused]] AutoObject<Recording> object_0) {
    const AutoObject<Recording> object_2;
    Baz();
}

void Foo(AutoObject<Recording> object_0,
         [[maybe_unused]] AutoObject<Recording> object_1) {
    Bar(object_0);
}

// NOLINTEND(performance-unnecessary-value-param)

TEST(Destructors, FunctionArguments) {
    Recording::Reset();

    TRY {
        const AutoObject<Recording> object_0;
        const AutoObject<Recording> object_1;
        Foo(object_0, object_1);
    }
    CATCH(RuntimeError) {
        const std::vector<size_t> expected{
            2,  // local in Bar
            0,  // argument of Bar
            1,  // second argument of Foo
            0,  // first argument of Foo
            1,  // local object_1 in TRY
            0,  // local object_0 in TRY
        };
        ASSERT_EQ(Recording::destructors, expected);
    }
}

// NOLINTEND(readability-function-cognitive-complexity)

}  // namespace except::test::impl
