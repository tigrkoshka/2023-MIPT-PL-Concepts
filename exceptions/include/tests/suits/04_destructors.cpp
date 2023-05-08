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
            } CATCH(Exception) {
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

            expected.push_back(5);
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

// NOLINTEND(readability-function-cognitive-complexity)

}  // namespace except::test::impl
