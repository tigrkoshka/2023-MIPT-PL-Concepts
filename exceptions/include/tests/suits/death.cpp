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

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
struct ThrowingDestructor {
    ~ThrowingDestructor() {
        Throw(RuntimeError{});
    }
};

TEST(Death, DuringUnwinding) {
    TRY {
        const AutoObject<ThrowingDestructor> object;

        TRY {
            // this is OK, the object is not affected by stack unwinding
            Throw(Exception{});
        }
        CATCH() {
            // OK
        }

        // this results in std::terminate since the stack unwinding attempts
        // to destroy the object, whose destructor throws another exception
        ASSERT_DEATH(Throw(Exception{}), "");
    }
    CATCH() {}
}

// NOLINTEND(readability-function-cognitive-complexity)

}  // namespace except::test::impl
