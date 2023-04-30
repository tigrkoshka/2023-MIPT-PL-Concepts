#include "exception_stack.hpp"

namespace except::detail {

thread_local std::stack<Node*> stack{};

} // namespace except::detail
