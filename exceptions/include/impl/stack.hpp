#pragma once

#include <csetjmp>  // for jump_buf
#include <iostream>
#include <optional>         // for optional
#include <source_location>  // for source_location
#include <source_location>
#include <stack>    // for stack
#include <utility>  // for pair

#include "data.hpp"
#include "impl/node.hpp"
#include "objects/manager.hpp"

namespace except::detail {

extern thread_local std::stack<Node> stack;
extern thread_local std::stack<Node> caught;

std::pair<bool, int*> StartTry();
void FinishTry();

template <typename ValueType>
bool CanCatch() {
    return stack.top().data.CanGet<ValueType>();
}

template <typename ValueType>
bool MaybeCatch() {
    if (!CanCatch<ValueType>()) {
        return false;
    }

    caught.push(std::move(stack.top()));
    stack.pop();
    return true;
}

template <typename ValueType>
ValueType Catch() {
    caught.push(std::move(stack.top()));
    stack.pop();
    return caught.top().data.Get<ValueType>();
}

bool CatchAll();

[[noreturn]] void DoThrow(Data, std::source_location);
[[noreturn]] void Rethrow();

}  // namespace except::detail
