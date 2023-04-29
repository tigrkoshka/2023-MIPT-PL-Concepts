#include "utils.hpp"

#include <csetjmp>    // for longjmp
#include <cstddef>    // for size_t
#include <exception>  // for terminate
#include <iostream>   // for cerr, endl
#include <string>     // for string
#include <utility>    // for move, exchange

#include "exception.hpp"
#include "object_manager.hpp"

namespace except::details {

thread_local Node* top;

void Push(Node* exception) {
    exception->prev = std::exchange(top, exception);
}

void Pop() {
    top = top->prev;
}

void Throw(Type type, std::string file, size_t line) {
    if (top == nullptr) {
        std::cerr << "uncaught exception of type " << Message(type)
                  << " at line " << line << " of file " << file << std::endl;
        std::terminate();
    }

    Node* current = std::exchange(top, top->prev);

    current->type = type;
    current->file = std::move(file);
    current->line = line;

    ObjectManager::UnwindToCheckpoint();

    // NOLINTNEXTLINE(cert-err52-cpp)
    std::longjmp(static_cast<int*>(current->buf),
                 static_cast<int>(Status::RAISED));
}

}  // namespace except::details
