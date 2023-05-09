#include <gtest/gtest.h>

#include "objects/auto.hpp"
#include "objects/tests/suits/fixture.hpp"
#include "objects/tests/utils/concepts.hpp"

namespace except::test::objects {

// NOLINTBEGIN(readability-function-cognitive-complexity)

TYPED_TEST(AutoTest, Methods) {
    using Plain = TestFixture::Plain;

    // both const and non-const methods are available for non-const instances

    AutoObject<Plain> sample = kValueS;
    ASSERT_EQ(sample->Index(), kValueS.size());
    sample->IncrementIndex();
    ASSERT_EQ(sample->Index(), kValueS.size() + 1);

    // only const methods are available on const instances

    const AutoObject<Plain> const_sample = kValueS;
    ASSERT_EQ(const_sample->Index(), kValueS.size());
    // const_sample.IncrementIndex();

    // static checks

    static_assert(CanIndex<AutoObject<Plain>>);
    static_assert(CanIndex<const AutoObject<Plain>>);

    static_assert(CanIncrementIndex<AutoObject<Plain>>);
    static_assert(not CanIncrementIndex<const AutoObject<Plain>>);
}

// NOLINTEND(readability-function-cognitive-complexity)

}  // namespace except::test::objects
