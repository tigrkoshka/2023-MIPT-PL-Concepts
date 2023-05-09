#include "impl.hpp"

#include <csetjmp>          // for longjmp
#include <exception>        // for terminate
#include <iostream>         // for cerr, endl
#include <source_location>  // for source_location
#include <stack>            // for stack
#include <utility>          // for move
#include <vector>           // for vector

#include "impl/data.hpp"
#include "impl/node.hpp"
#include "objects/manager.hpp"

namespace except::detail {

int* Impl::StartTry() {
    stack.emplace(stack.size());
    ObjectManager::RecordCheckpoint();
    return stack.top().Buff();
}

void Impl::FinishTry() {
    if (stack.empty()) {
        caught.pop();
        return;
    }

    if (stack.top().raised) {
        // same as Catch(); Rethrow();

        Node thrown = std::move(stack.top());
        stack.pop();

        Throw(std::move(thrown.data), thrown.source_location);
    }

    if (!caught.empty() && stack.top().depth < caught.top().depth) {
        caught.pop();
    } else {
        ObjectManager::Pop();
        stack.pop();
    }
}

void Impl::DoCatch() {
    caught.emplace(std::move(stack.top()));
    stack.pop();
}

bool Impl::CatchAll() {
    DoCatch();
    return true;
}

void Impl::Throw(Data data, std::source_location source_location) {
    if (unwinding_in_progress) {
        // safe to call stack.top() since the unwinding_in_progress flag is set
        // to true only inside this function and only after checking that
        // the stack is not empty

        std::cerr << "an exception occurred at line " << source_location.line()
                  << " of file " << source_location.file_name() << std::endl
                  << "during stack unwinding as a part of processing "
                  << "exception that occurred at line "
                  << stack.top().source_location.line() << " of file "
                  << stack.top().source_location.file_name() << std::endl;
        std::terminate();
    }

    if (stack.empty()) {
        std::cerr << "uncaught exception at line " << source_location.line()
                  << " of file " << source_location.file_name() << std::endl;
        std::terminate();
    }

    while (!caught.empty() && stack.top().depth < caught.top().depth) {
        caught.pop();
    }

    // save the exception before the stack unwinding begins
    // to be able to output more logs if the stack unwinding fails
    stack.top().Raise(std::move(data), source_location);

    unwinding_in_progress = true;
    ObjectManager::UnwindToCheckpoint();
    unwinding_in_progress = false;

    std::longjmp(stack.top().Buff(), 1);  // NOLINT(cert-err52-cpp)
}

void Impl::Throw() {
    if (caught.empty()) {
        std::cerr << "attempt to rethrow an exception outside of a CATCH block"
                  << std::endl;
        std::terminate();
    }

    // copy data, because this exception might be reused later
    Throw(caught.top().data, caught.top().source_location);
}

thread_local std::stack<Node, std::vector<Node>> Impl::stack;
thread_local std::stack<Node, std::vector<Node>> Impl::caught;
thread_local bool Impl::unwinding_in_progress = false;

}  // namespace except::detail
