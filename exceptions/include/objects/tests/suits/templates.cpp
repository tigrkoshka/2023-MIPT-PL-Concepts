#include <gtest/gtest.h>

#include <concepts>  // for same_as, ...
#include <string>    // for string
#include <vector>    // for vector

#include "objects/auto.hpp"
#include "objects/tests/utils/concepts.hpp"

namespace except::test::objects {

struct SomeStruct {};

// NOLINTBEGIN(readability-function-cognitive-complexity)

TEST(Template, Concept) {
    static_assert(                           //
        CanAutoObject<SomeStruct> &&         //
        CanAutoObject<std::string> &&        //
        CanAutoObject<std::vector<int>> &&   //
        CanAutoObject<decltype([](int) {})>  // lambda is a class
    );

    static_assert(                   //
        not CanAutoObject<int> &&    // not a class
        not CanAutoObject<int*> &&   // not a class
        not CanAutoObject<int&> &&   // not a class
        not CanAutoObject<int&&> &&  // not a class

        // NOLINTNEXTLINE(*-avoid-c-arrays)
        not CanAutoObject<char[]> &&  // not a class
        // NOLINTNEXTLINE(*-avoid-c-arrays)
        not CanAutoObject<char[4]> &&            // not a class
        not CanAutoObject<void (*)(double)> &&   // not a class
        not CanAutoObject<void(std::string)> &&  // not a class

        not CanAutoObject<SomeStruct*> &&           // not a class
        not CanAutoObject<const SomeStruct****> &&  // not a class
        not CanAutoObject<SomeStruct&> &&           // not a class
        not CanAutoObject<SomeStruct&&> &&          // not a class

        not CanAutoObject<const SomeStruct> &&           // cv-qualified
        not CanAutoObject<volatile SomeStruct> &&        // cv-qualified
        not CanAutoObject<const volatile SomeStruct> &&  // cv-qualified

        not CanAutoObject<const std::string> &&        // cv-qualified
        not CanAutoObject<volatile std::string> &&     // cv-qualified
        not CanAutoObject<const volatile std::string>  // cv-qualified
    );
}

TEST(Template, Deduction) {
    const AutoObject sample_1 = [](int) {};

    // because different classes are produced for each lambda
    static_assert(not std::same_as<decltype(sample_1),
                                   const AutoObject<decltype([](int) {})>>);

    auto lambda_1             = [](int) {};
    const AutoObject sample_2 = lambda_1;

    // because different classes are produced for each lambda
    static_assert(not std::same_as<decltype(sample_1), decltype(sample_2)>);
    // here the exact lambda object is used in decltype
    static_assert(std::same_as<decltype(sample_2),  //
                               const AutoObject<decltype(lambda_1)>>);

    auto lambda_2             = [](int) {};
    const AutoObject sample_3 = lambda_2;

    // because different classes are produced for each lambda
    static_assert(not std::same_as<decltype(sample_2), decltype(sample_3)>);
    // here the exact lambda object is used in decltype
    static_assert(std::same_as<decltype(sample_3),  //
                               const AutoObject<decltype(lambda_2)>>);

    const AutoObject sample_4 = sample_3;

    // but when copying, the type is preserved
    static_assert(std::same_as<decltype(sample_3), decltype(sample_4)>);
    // here the exact lambda object is used in decltype
    static_assert(std::same_as<decltype(sample_4),  //
                               const AutoObject<decltype(lambda_2)>>);
}

// NOLINTEND(readability-function-cognitive-complexity)

}  // namespace except::test::objects
