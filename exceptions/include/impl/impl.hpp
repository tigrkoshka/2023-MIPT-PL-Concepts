#pragma once

#include <csetjmp>          // for jump_buf
#include <source_location>  // for source_location
#include <stack>            // for stack
#include <utility>          // for pair
#include <vector>           // for vector

#include "impl/node.hpp"
#include "utils/concepts.hpp"

namespace except::detail {

class Impl final {
   private:
    static void DoCatch();

   public:
    Impl() = delete;

    static int* StartTry();
    static void FinishTry();

    template <typename ValueType>
    static bool CanCatch() {
        return stack.top().data.CanGet<ValueType>();
    }

    template <utils::concepts::Catchable ValueType>
    static bool MaybeCatch() {
        if (!CanCatch<ValueType>()) {
            return false;
        }

        DoCatch();
        return true;
    }

    template <utils::concepts::Catchable ValueType>
    static std::pair<ValueType, bool> Catch() {
        DoCatch();
        return {caught.top().data.Get<ValueType>(), true};
    }

    static bool CatchAll();

    [[noreturn]] static void Throw(Data, std::source_location);
    [[noreturn]] static void Rethrow();

   private:
    // TODO: the second template argument is a workaround for a GCC bug,
    //       which fails with SIGSEGV on trying to insert an element
    //       in a thread_local std::deque (which is the default second
    //       template parameter for std::stack)
    //       track a similar issue here: https://clck.ru/34Ld6E
    //       once the bug is fixed, the second template parameter
    //       can be removed
    static thread_local std::stack<Node, std::vector<Node>> stack;
    static thread_local std::stack<Node, std::vector<Node>> caught;
};

}  // namespace except::detail
