#pragma once

#include <compare>
#include <cstddef>
#include <ostream>

namespace except::test::objects {

struct Stats {
    struct Constructors {
        // NOLINTNEXTLINE(fuchsia-overloaded-operator)
        std::strong_ordering operator<=>(const Constructors&) const = default;
        Constructors& operator+=(const Constructors&);

        size_t default_{0};
        size_t copy{0};
        size_t move{0};
        size_t single_argument{0};
        size_t multi_argument{0};
    };

    struct Assignments {
        // NOLINTNEXTLINE(fuchsia-overloaded-operator)
        std::strong_ordering operator<=>(const Assignments&) const = default;
        Assignments& operator+=(const Assignments&);

        size_t copy{0};
        size_t move{0};
    };

    // NOLINTNEXTLINE(fuchsia-overloaded-operator)
    std::strong_ordering operator<=>(const Stats&) const = default;
    Stats& operator+=(const Stats&);

    Constructors ctor;
    Assignments assign;
    size_t dtor{0};
};

// NOLINTNEXTLINE(fuchsia-overloaded-operator)
std::ostream& operator<<(std::ostream& out, const Stats::Constructors& ctor);

// NOLINTNEXTLINE(fuchsia-overloaded-operator)
std::ostream& operator<<(std::ostream& out, const Stats::Assignments& assign);

// NOLINTNEXTLINE(fuchsia-overloaded-operator)
std::ostream& operator<<(std::ostream& out, const Stats& stats);

void ResetStats();

}  // namespace except::test::objects
