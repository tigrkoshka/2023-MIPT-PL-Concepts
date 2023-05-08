#include "stats.hpp"

#include <cstddef>
#include <ostream>

#include "objects/tests/utils/non_trivial.hpp"

namespace except::test::objects::detail {

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

}  // namespace except::test::objects::detail
