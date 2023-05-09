#include <gtest/gtest.h>

#include <utility> // for move

#include "objects/auto.hpp"
#include "objects/tests/suits/fixture.hpp"
#include "objects/tests/utils/concepts.hpp"

namespace except::test::objects {

// NOLINTBEGIN(performance-unnecessary-value-param)
void AutoValue(AutoObject<SimplePlain>) {}
void AutoValue(AutoObject<FinalPlain>) {}

void PlainValue(SimplePlain) {}
void PlainValue(FinalPlain) {}

void AutoConstValue(const AutoObject<SimplePlain>) {}
void AutoConstValue(const AutoObject<FinalPlain>) {}

void PlainConstValue(const SimplePlain) {}
void PlainConstValue(const FinalPlain) {}
// NOLINTEND(performance-unnecessary-value-param)

// no need for AutoObject since no objects are being constructed
void PlainRef(SimplePlain&) {}
void PlainRef(FinalPlain&) {}

// no need for AutoObject since no objects are being constructed
void PlainConstRef(const SimplePlain&) {}
void PlainConstRef(const FinalPlain&) {}

void AutoRefRef(AutoObject<SimplePlain>&&) {}
void AutoRefRef(AutoObject<FinalPlain>&&) {}

void PlainRefRef(SimplePlain&&) {}
void PlainRefRef(FinalPlain&&) {}

void PlainAmbiguous(SimplePlain&&) {}
void PlainAmbiguous(const SimplePlain&) {}
void PlainAmbiguous(FinalPlain&&) {}
void PlainAmbiguous(const FinalPlain&) {}

void AutoAmbiguous(AutoObject<SimplePlain>&&) {}
void AutoAmbiguous(const AutoObject<SimplePlain>&) {}
void AutoAmbiguous(AutoObject<FinalPlain>&&) {}
void AutoAmbiguous(const AutoObject<FinalPlain>&) {}

// NOLINTBEGIN(bugprone-use-after-move,hicpp-invalid-access-moved,readability-function-cognitive-complexity)
TYPED_TEST(AutoTest, Usage) {
    using Plain = TestFixture::Plain;

    ResetStats();

    Plain plain;
    AutoObject<Plain> object;
    const Plain const_plain;
    const AutoObject<Plain> const_object;

    Stats expected{.ctor{.default_ = 4}};
    ASSERT_EQ(Plain::stats, expected);

    ////////////////////////////////////////////////////////////////////////////
    ///                      Renew values after move                         ///
    ////////////////////////////////////////////////////////////////////////////

    auto renew = [&]() {
        plain  = Plain{};
        object = AutoObject<Plain>{};

        expected += Stats{.ctor{.default_ = 2}};  // constructing temporaries
        expected += Stats{.assign{.move = 2}};
        expected += Stats{.dtor = 2};  // destroying temporaries
        ASSERT_EQ(Plain::stats, expected);
    };

    ////////////////////////////////////////////////////////////////////////////
    ///                Passing by value (requires a copy)                    ///
    ////////////////////////////////////////////////////////////////////////////

    AutoValue(plain);
    AutoValue(object);
    PlainValue(plain);
    PlainValue(object);

    AutoValue(const_plain);
    AutoValue(const_object);
    PlainValue(const_plain);
    PlainValue(const_object);

    AutoConstValue(plain);
    AutoConstValue(object);
    PlainConstValue(plain);
    PlainConstValue(object);

    AutoConstValue(const_plain);
    AutoConstValue(const_object);
    PlainConstValue(const_plain);
    PlainConstValue(const_object);

    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    expected += Stats{.ctor{.copy = 16}};
    expected += Stats{.dtor = 16};  // function arguments get destroyed
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    ASSERT_EQ(Plain::stats, expected);

    ////////////////////////////////////////////////////////////////////////////
    ///                    Passing by lvalue reference                       ///
    ////////////////////////////////////////////////////////////////////////////

    PlainRef(plain);
    PlainRef(object);

    // cannot pass const by non-const ref
    // PlainRef(const_plain);
    // PlainRef(const_object);

    PlainConstRef(plain);
    PlainConstRef(object);

    PlainConstRef(const_plain);
    PlainConstRef(const_object);

    // no new objects get constructed when passing by lvalue reference
    ASSERT_EQ(Plain::stats, expected);

    ////////////////////////////////////////////////////////////////////////////
    ///               Passing by rvalue reference to Plain                   ///
    ////////////////////////////////////////////////////////////////////////////

    // no new objects are constructed here
    PlainRefRef(std::move(plain));
    // no new objects are constructed here,
    // because AutoObject<Plain>&& can be implicitly converted to Plain&&,
    // and the conversion does not require constructing a new object
    // (it just moves the instance stored inside of it)
    PlainRefRef(std::move(object));

    // cannot move a const value
    // PlainRefRef(std::move(const_plain));
    // PlainRefRef(std::move(const_object));

    // no constructors are called
    ASSERT_EQ(Plain::stats, expected);

    renew();

    ////////////////////////////////////////////////////////////////////////////
    ///                Passing by rvalue reference to Auto                   ///
    ////////////////////////////////////////////////////////////////////////////

    // here AutoObject gets constructed and then destroyed
    AutoRefRef(std::move(plain));
    // no new objects are constructed here
    AutoRefRef(std::move(object));

    // cannot move a const value
    // AutoRefRef(std::move(const_plain));
    // AutoRefRef(std::move(const_object));

    expected += Stats{.ctor{.move = 1}};
    expected += Stats{.dtor = 1};
    ASSERT_EQ(Plain::stats, expected);

    renew();

    ////////////////////////////////////////////////////////////////////////////
    ///                      Passing rvalue as value                         ///
    ////////////////////////////////////////////////////////////////////////////

    AutoValue(std::move(plain));
    AutoValue(std::move(object));

    expected += Stats{.ctor{.move = 2}};
    expected += Stats{.dtor = 2};
    ASSERT_EQ(Plain::stats, expected);

    renew();

    PlainValue(std::move(plain));
    PlainValue(std::move(object));

    expected += Stats{.ctor{.move = 2}};
    expected += Stats{.dtor = 2};
    ASSERT_EQ(Plain::stats, expected);

    renew();

    AutoConstValue(std::move(plain));
    AutoConstValue(std::move(object));

    expected += Stats{.ctor{.move = 2}};
    expected += Stats{.dtor = 2};
    ASSERT_EQ(Plain::stats, expected);

    renew();

    PlainConstValue(std::move(plain));
    PlainConstValue(std::move(object));

    expected += Stats{.ctor{.move = 2}};
    expected += Stats{.dtor = 2};
    ASSERT_EQ(Plain::stats, expected);

    renew();

    ////////////////////////////////////////////////////////////////////////////
    ///             Passing rvalue as const lvalue reference                 ///
    ////////////////////////////////////////////////////////////////////////////

    // no objects get constructed here (temporary binds to const lvalue ref)
    PlainConstRef(std::move(plain));
    // no objects get constructed here (temporary binds to const lvalue ref)
    PlainConstRef(std::move(object));

    ASSERT_EQ(Plain::stats, expected);

    renew();

    ////////////////////////////////////////////////////////////////////////////
    ///                          Ambiguous call                              ///
    ////////////////////////////////////////////////////////////////////////////

    // constructs AutoObject from Plain&& and chooses
    // AutoAmbiguous(AutoObject<Plain>&&), then deletes the temporary
    AutoAmbiguous(std::move(plain));
    // OK, chooses AutoAmbiguous(AutoObject<Plain>&&),
    // no objects are constructed
    AutoAmbiguous(std::move(object));

    expected += Stats{.ctor{.move = 1}};
    expected += Stats{.dtor = 1};
    ASSERT_EQ(Plain::stats, expected);

    renew();

    // chooses PlainAmbiguous(Plain&&)
    PlainAmbiguous(std::move(plain));
    if constexpr (std::is_same_v<Plain, SimplePlain>) {
        // OK, chooses PlainAmbiguous(Plain&&)
        PlainAmbiguous(std::move(object));
    } else {
        // Ambiguous call (Plain is FinalPlain):
        //   1) move gives AutoObject<Plain>&&
        //   2) the compiler considers the PlainAmbiguous overloads:
        //        if we are to call PlainAmbiguous(Plain&&), we need to
        //        implicitly convert const AutoObject<Plain>& to const Plain&
        //        (by binding AutoObject<Plain>&& to const AutoObject<Plain>&
        //        and calling the const lvalue ref conversion operator)
        //
        //        if we are to call PlainAmbiguous(Plain&&), we need to
        //        implicitly convert AutoObject<Plain>&& to const Plain&&
        //        (calling the rvalue conversion operator)
        //
        //      both paths have a single implicit conversion,
        //      so they are equally good, thus the ambiguity
        // PlainAmbiguous(std::move(object));

        // a special method Move() && is defined specifically for this case,
        // it always returns Plain&& to disambiguate the call by forcing
        // PlainAmbiguous(Plain&&)
        PlainAmbiguous(std::move(object).Move());

        // Note: in C++23 it is possible to use perfect forwarding on
        // the instance, for which the method is called, which would allow us
        // to get rid of this workaround
    }

    ASSERT_EQ(Plain::stats, expected);

    renew();
}
// NOLINTEND(bugprone-use-after-move,hicpp-invalid-access-moved,readability-function-cognitive-complexity)

}  // namespace except::test::objects
