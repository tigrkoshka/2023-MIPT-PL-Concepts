#include <gtest/gtest.h>

#include <cstddef>  // for size_t

//
#include <except>

namespace except::test::impl {

// NOLINTBEGIN(readability-function-cognitive-complexity)

TEST(Death, Simple) {
    ASSERT_DEATH(Throw(Exception{}), "");
}

TEST(Death, InCatch) {
    TRY {
        Throw(Exception{});
    }
    CATCH(Exception) {
        ASSERT_DEATH(Throw(Exception{}), "");
        ASSERT_DEATH(Throw(), "");
    }
}

TEST(Death, Hard) {
    TRY {
        TRY {
            TRY {
                TRY {
                    Throw(Exception{});
                }
                CATCH(Exception) {
                    Throw();
                }
            }
            CATCH(Exception) {
                // OK
            }

            Throw(Exception{});
        }
        CATCH(Exception) {
            TRY {
                // OK
            }
            CATCH() {
                // should not enter
                ASSERT_TRUE(false);
            }
        }

        ASSERT_DEATH(Throw(), "");
        Throw(Exception{});
    }
    CATCH(Exception) {
        ASSERT_DEATH(Throw(Exception{}), "");
        ASSERT_DEATH(Throw(), "");
    }
}

// NOLINTEND(readability-function-cognitive-complexity)

}  // namespace except::test::impl
