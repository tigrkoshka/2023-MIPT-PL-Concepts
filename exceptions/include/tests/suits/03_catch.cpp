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

// NOLINTEND(readability-function-cognitive-complexity)

}  // namespace except::test::impl
