#include <gtest/gtest.h>

#include <string>  // for string

//
#include <except>

#include "tests/utils/custom.hpp"

namespace except::test::impl {

// NOLINTBEGIN(readability-function-cognitive-complexity)

TEST(Catch, ByValue) {
    TRY {
        TRY {
            Throw(std::string("Exception"));
        }
        CATCH(std::string, exception) {
            exception = "Other exception";
            Throw();
        }
    }
    CATCH(std::string, exception) {
        ASSERT_EQ(exception, "Exception");
    }
}

TEST(Catch, ConstByValue) {
    const std::string value = "Exception";

    TRY {
        TRY {
            Throw(value);  // a copy occurs here, so const-ness is lost
        }
        CATCH(std::string, exception) {
            exception = "Other exception";
            Throw();
        }
    }
    CATCH(std::string, exception) {
        ASSERT_EQ(exception, "Exception");
    }
}

TEST(Catch, ByReference) {
    TRY {
        TRY {
            Throw(std::string("Exception"));
        }
        CATCH(std::string&, exception) {
            exception = "Other exception";
            Throw();
        }
    }
    CATCH(std::string, exception) {
        ASSERT_EQ(exception, "Other exception");
    }
}

TEST(Catch, ConstByReference) {
    const std::string value = "Exception";

    TRY {
        TRY {
            Throw(value);  // a copy occurs here, so const-ness is lost
        }
        CATCH(std::string&, exception) {
            exception = "Other exception";
            Throw();
        }
    }
    CATCH(std::string, exception) {
        ASSERT_EQ(exception, "Other exception");
    }
}

TEST(Catch, CustomByReference) {
    TRY {
        TRY {
            Throw(CustomExceptionChild{"Exception"});
        }
        CATCH(CustomExceptionChild&, exception) {
            exception.message = "Other exception";
            Throw();
        }
    }
    CATCH(CustomExceptionChild, exception) {
        ASSERT_EQ(exception.message, "Other exception");
    }
}

TEST(Catch, CustomByReferenceToParent) {
    TRY {
        TRY {
            Throw(CustomExceptionChild{"Exception"});
        }
        CATCH(CustomExceptionParent&, exception) {
            exception.message = "Other exception";
            Throw();
        }
    }
    CATCH(CustomExceptionChild&, exception) {
        ASSERT_EQ(exception.message, "Other exception");
    }
}

TEST(Catch, CatchAll) {
    TRY {
        Throw(std::string("Exception"));
    }
    CATCH(CustomExceptionChild) {
        // should not enter
        ASSERT_TRUE(false);
    }
    CATCH(CustomExceptionParent) {
        // should not enter
        ASSERT_TRUE(false);
    }
    CATCH(Exception) {
        // should not enter
        ASSERT_TRUE(false);
    }
    CATCH(const char*) {
        // should not enter
        ASSERT_TRUE(false);
    }
    CATCH() {
        // OK
    }
}

// NOLINTBEGIN(*-avoid-c-arrays)
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

TEST(Catch, WeirdTypesUnboundedArray) {
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

TEST(Catch, WeirdTypesBoundedArray) {
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
