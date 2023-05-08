#include "stats.hpp"

#include <cstddef>
#include <ostream>

#include "objects/tests/utils/sample_class.hpp"

namespace except::test::objects {

auto Stats::Constructors::operator+=(const Constructors& other)
    -> Constructors& {
    default_ += other.default_;
    copy += other.copy;
    move += other.move;
    single_argument += other.single_argument;
    multi_argument += other.multi_argument;

    return *this;
}

auto Stats::Assignments::operator+=(const Assignments& other) -> Assignments& {
    copy += other.copy;
    move += other.move;

    return *this;
}

auto Stats::operator+=(const Stats& other) -> Stats& {
    ctor += other.ctor;
    assign += other.assign;
    dtor += other.dtor;

    return *this;
}

// NOLINTNEXTLINE(fuchsia-overloaded-operator)
std::ostream& operator<<(std::ostream& out, const Stats::Constructors& ctor) {
    return out << "    default:_________" << ctor.default_ << std::endl
               << "    copy:____________" << ctor.copy << std::endl
               << "    move:____________" << ctor.move << std::endl
               << "    single_argument:_" << ctor.single_argument << std::endl
               << "    multi_argument:__" << ctor.multi_argument;
}

// NOLINTNEXTLINE(fuchsia-overloaded-operator)
std::ostream& operator<<(std::ostream& out, const Stats::Assignments& assign) {
    return out << "    copy:____________" << assign.copy << std::endl
               << "    move:____________" << assign.move;
}

// NOLINTNEXTLINE(fuchsia-overloaded-operator)
std::ostream& operator<<(std::ostream& out, const Stats& stats) {
    return out << "constructors:" << std::endl
               << stats.ctor << std::endl
               << "assignments:" << std::endl
               << stats.assign << std::endl
               << "destructors: " << stats.dtor;
}

void ResetStats() {
    SimplePlain::stats    = Stats{};
    SimpleExplicit::stats = Stats{};
    SimpleNoexcept::stats = Stats{};

    FinalPlain::stats    = Stats{};
    FinalExplicit::stats = Stats{};
    FinalNoexcept::stats = Stats{};
}

}  // namespace except::test::objects
