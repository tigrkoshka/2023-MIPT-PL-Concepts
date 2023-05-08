#include <gtest/gtest.h>

#include "objects/auto.hpp"
#include "objects/tests/utils/concepts.hpp"
#include "objects/tests/utils/non_trivial.hpp"
#include "objects/tests/utils/stats.hpp"

namespace except::test::objects {

using namespace detail;

#include "templates.hpp"

namespace simple {

using Plain    = SimplePlain;
using Explicit = SimpleExplicit;
using Noexcept = SimpleNoexcept;
#define DOT .

#include "assignments.hpp"
#include "constructors.hpp"
#include "conversions.hpp"
#include "destructor.hpp"
#include "methods.hpp"
#include "operators.hpp"

#undef DOT

}  // namespace simple

namespace final {

using Plain    = FinalPlain;
using Explicit = FinalExplicit;
using Noexcept = FinalNoexcept;
#define DOT ->

#define FINAL
#include "assignments.hpp"
#include "constructors.hpp"
#include "conversions.hpp"
#include "destructor.hpp"
#include "methods.hpp"
#include "operators.hpp"
#undef FINAL

#undef DOT

}  // namespace final

}  // namespace except::test::objects
