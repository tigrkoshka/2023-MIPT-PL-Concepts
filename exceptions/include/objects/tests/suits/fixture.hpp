#include <gtest/gtest.h>

#include <concepts>     // for same_as
#include <string>       // for string
#include <type_traits>  // for conditional_t

#include "objects/tests/utils/sample_class.hpp"

namespace except::test::objects {

struct MakeSimple {};
struct MakeFinal {};

template <typename T>
    requires(std::same_as<MakeSimple, T> || std::same_as<MakeFinal, T>)
class AutoTest : public ::testing::Test {
   public:
    using Plain = std::conditional_t<std::same_as<T, MakeFinal>,  //
                                     FinalPlain,
                                     SimplePlain>;

    using Explicit = std::conditional_t<std::same_as<T, MakeFinal>,
                                        FinalExplicit,
                                        SimpleExplicit>;

    using Noexcept = std::conditional_t<std::same_as<T, MakeFinal>,
                                        FinalNoexcept,
                                        SimpleNoexcept>;
};

using TestTypes = ::testing::Types<MakeSimple, MakeFinal>;
TYPED_TEST_SUITE(AutoTest, TestTypes);

static const char* value_c            = "value";
static const char* const_value_c      = "const value";
static const std::string kValueS      = value_c;
static const std::string kConstValueS = const_value_c;
static const size_t kValueSZ          = 239ull;

}  // namespace except::test::objects
