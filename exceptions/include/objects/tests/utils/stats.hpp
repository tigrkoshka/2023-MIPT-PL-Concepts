#pragma once

#include <compare>
#include <cstddef>
#include <ostream>

namespace except::test::objects {

// NOLINTBEGIN(fuchsia-overloaded-operator)

struct Stats {
    struct Constructors {
        std::strong_ordering operator<=>(const Constructors&) const = default;
        Constructors& operator+=(const Constructors&);

        size_t default_{0};
        size_t copy{0};
        size_t move{0};
        size_t single_argument{0};
        size_t multi_argument{0};
    };

    struct Assignments {
        std::strong_ordering operator<=>(const Assignments&) const = default;
        Assignments& operator+=(const Assignments&);

        size_t copy{0};
        size_t move{0};
    };

    std::strong_ordering operator<=>(const Stats&) const = default;
    Stats& operator+=(const Stats&);

    Constructors ctor;
    Assignments assign;
    size_t dtor{0};
};

std::ostream& operator<<(std::ostream& out, const Stats::Constructors& ctor);

std::ostream& operator<<(std::ostream& out, const Stats::Assignments& assign);

std::ostream& operator<<(std::ostream& out, const Stats& stats);

// NOLINTEND(fuchsia-overloaded-operator)

void ResetStats();

}  // namespace except::test::objects
