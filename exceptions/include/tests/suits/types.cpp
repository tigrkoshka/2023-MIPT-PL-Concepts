#include <gtest/gtest.h>

#include <cstddef>  // for size_t
#include <string>   // for string

//
#include <except>

#include "tests/utils/custom.hpp"

namespace except::test::impl {

// NOLINTBEGIN(readability-function-cognitive-complexity)

TEST(Types, NonException1) {
    TRY {
        Throw(size_t{0});
    }
    CATCH(int) {
        // should not enter
        ASSERT_TRUE(false);
    }
    CATCH(size_t) {
        // OK
    }
}

TEST(Types, NonException2) {
    size_t caught = 0;

    TRY {
        TRY {
            Throw(std::string{"Exception!"});
        }
        CATCH(const std::string&) {
            ++caught;

            TRY {
                Throw(size_t{0});
            }
            CATCH(int&) {
                // should not enter
                ASSERT_TRUE(false);
            }
            CATCH(size_t&) {
                ++caught;
            }

            Throw();
        }
    }
    CATCH(std::string) {
        ++caught;
    }

    ASSERT_EQ(caught, 3);
}

TEST(Types, NonException3) {
    TRY {
        Throw("Hello!");
    }
    CATCH(std::string) {
        // should not enter
        ASSERT_TRUE(false);
    }
    CATCH(const std::string&) {
        // should not enter
        ASSERT_TRUE(false);
    }
    CATCH(char*) {
        // should not enter
        ASSERT_TRUE(false);
    }
    CATCH(const char*) {
        // OK
    }
}

TEST(Types, ExceptionSimple) {
    TRY {
        Throw(Exception{});
    }
    CATCH(RuntimeError) {
        // should not enter
        ASSERT_TRUE(false);
    }
    CATCH(LogicError) {
        // should not enter
        ASSERT_TRUE(false);
    }
    CATCH(Exception) {
        // OK
    }
}

TEST(Types, ExceptionSibling) {
    TRY {
        Throw(RuntimeError{});
    }
    CATCH(LogicError) {
        // should not enter
        ASSERT_TRUE(false);
    }
    CATCH(const RuntimeError) {
        // OK
    }
}

TEST(Types, ExceptionChild) {
    TRY {
        Throw(RuntimeError{});
    }
    CATCH(RangeError) {
        // should not enter
        ASSERT_TRUE(false);
    }
    CATCH(Exception&) {
        // OK
    }
}

TEST(Types, ExceptionParent) {
    TRY {
        Throw(RuntimeError{});
    }
    CATCH(Exception) {
        // OK
    }
}

TEST(Types, ExceptionGrandParent) {
    TRY {
        Throw(RangeError{});
    }
    CATCH(const Exception&) {
        // OK
    }
}

TEST(Types, ExceptionCustomOther) {
    TRY {
        Throw(CustomExceptionParent{"Exception"});
    }
    CATCH(CustomExceptionChild) {
        // should not enter
        ASSERT_TRUE(false);
    }
    CATCH(CustomExceptionOther) {
        // should not enter
        ASSERT_TRUE(false);
    }
    CATCH(CustomExceptionParent) {
        // OK
    }
}

TEST(Types, ExceptionCustomToException) {
    TRY {
        Throw(CustomExceptionParent{"Exception"});
    }
    CATCH(Exception) {
        // OK
    }
}

TEST(Types, ExceptionCustomParent) {
    TRY {
        Throw(CustomExceptionChild{"Exception"});
    }
    CATCH(CustomExceptionParent) {
        // OK
    }
}

TEST(Types, ExceptionCustomGrandParent) {
    TRY {
        Throw(CustomExceptionChild{"Exception"});
    }
    CATCH(Exception) {
        // OK
    }
}

TEST(Types, ExceptionCustomSelf) {
    TRY {
        Throw(CustomExceptionParent{"Exception"});
    }
    CATCH(const CustomExceptionParent&, exception) {
        ASSERT_EQ(exception.message, "Exception");
    }
}

TEST(Types, ExceptionCustomParentPreservesValue) {
    TRY {
        Throw(CustomExceptionChild{"Exception"});
    }
    CATCH(const CustomExceptionParent&, exception) {
        ASSERT_EQ(exception.message, "Exception");
    }
}

TEST(Types, ExceptionCustomParentAfterRethrow) {
    TRY {
        TRY {
            Throw(CustomExceptionChild{"Exception"});
        }
        CATCH(const CustomExceptionParent&) {
            Throw();
        }
    }
    // catches even though we caught CustomExceptionParent before rethrowing
    CATCH(const CustomExceptionChild&) {
        // OK
    }
}

// NOLINTBEGIN(*-avoid-c-arrays)
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

TEST(Types, WeirdUnboundedArray) {
    char value[] = "Hello";

    ////////////////////////////////////////////////////////////////////////////

    TRY {
        Throw(value);
    }
    CATCH(char[], exception) {
        ASSERT_EQ(std::string(exception), "Hello");
    }

    /* works the same way
    try {
        throw value;
    } catch (char exception[]) {
        ASSERT_EQ(std::string(exception), "Hello");
    }
    */

    ////////////////////////////////////////////////////////////////////////////

    TRY {
        Throw(value);
    }
    CATCH(char[6], exception) {
        ASSERT_EQ(std::string(exception), "Hello");
    }

    /* works the same way
    try {
        throw value;
    } catch (char exception[6]) {
        ASSERT_EQ(std::string(exception), "Hello");
    }
    */

    ////////////////////////////////////////////////////////////////////////////

    TRY {
        Throw(value);
    }
    CATCH(char[2], exception) {
        ASSERT_EQ(std::string(exception), "Hello");
    }

    /* works the same way
    try {
        throw value;
    } catch (char exception[2]) {
        ASSERT_EQ(std::string(exception), "Hello");
    }
    */

    ////////////////////////////////////////////////////////////////////////////

    // constant pointer -- OK

    TRY {
        Throw(value);
    }
    CATCH(char* const, exception) {
        ASSERT_EQ(std::string(exception), "Hello");
    }

    /* works the same way
    try {
        throw value;
    } catch (const char exception[]) {
        ASSERT_EQ(std::string(exception), "Hello");
    }
    */

    ////////////////////////////////////////////////////////////////////////////

    // pointer to constant -- cannot catch

    TRY {
        Throw(value);
    }
    CATCH(const char[], exception) {
        // should not enter
        ASSERT_TRUE(false);
    }
    CATCH() {
        // OK
    }

    /* this is allowed though
    try {
        throw value;
    } catch (const char exception[]) {
        ASSERT_EQ(std::string(exception), "Hello");
    }
    */

    ////////////////////////////////////////////////////////////////////////////

    // modifying

    TRY {
        TRY {
            Throw(value);
        }
        CATCH(char[], exception) {
            exception[0] = 'L';
            Throw();
        }
    }
    CATCH(char[], exception) {
        ASSERT_EQ(std::string(exception), "Lello");
    }

    /* works the same way
    try {
        try {
            throw value;
        } catch (char exception[]) {
           exception[0] = 'L';
           throw;
        }
    } catch (char exception[]) {
        ASSERT_EQ(std::string(exception), "Lello");
    }
    */
}

TEST(Types, WeirdBoundedArray) {
    char value[6] = "Hello";

    ////////////////////////////////////////////////////////////////////////////

    TRY {
        Throw(value);
    }
    CATCH(char[], exception) {
        ASSERT_EQ(std::string(exception), "Hello");
    }

    /* works the same way
    try {
        throw value;
    } catch (char exception[]) {
        ASSERT_EQ(std::string(exception), "Hello");
    }
    */

    ////////////////////////////////////////////////////////////////////////////

    TRY {
        Throw(value);
    }
    CATCH(char[6], exception) {
        ASSERT_EQ(std::string(exception), "Hello");
    }

    /* works the same way
    try {
        throw value;
    } catch (char exception[6]) {
        ASSERT_EQ(std::string(exception), "Hello");
    }
    */

    ////////////////////////////////////////////////////////////////////////////

    TRY {
        Throw(value);
    }
    CATCH(char[2], exception) {
        ASSERT_EQ(std::string(exception), "Hello");
    }

    /* works the same way
    try {
        throw value;
    } catch (char exception[2]) {
        ASSERT_EQ(std::string(exception), "Hello");
    }
    */

    ////////////////////////////////////////////////////////////////////////////

    // constant pointer -- OK

    TRY {
        Throw(value);
    }
    CATCH(char* const, exception) {
        ASSERT_EQ(std::string(exception), "Hello");
    }

    /* works the same way
    try {
        throw value;
    } catch (const char exception[]) {
        ASSERT_EQ(std::string(exception), "Hello");
    }
    */

    ////////////////////////////////////////////////////////////////////////////

    // pointer to constant -- cannot catch

    TRY {
        Throw(value);
    }
    CATCH(const char[], exception) {
        // should not enter
        ASSERT_TRUE(false);
    }
    CATCH() {
        // OK
    }

    /* this is allowed though
    try {
        throw value;
    } catch (const char exception[]) {
        ASSERT_EQ(std::string(exception), "Hello");
    }
    */
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
// NOLINTEND(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
// NOLINTEND(*-avoid-c-arrays)

// NOLINTEND(readability-function-cognitive-complexity)

}  // namespace except::test::impl
