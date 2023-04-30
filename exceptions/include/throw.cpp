#include "throw.hpp"

#include <csetjmp>    // for longjmp
#include <cstddef>    // for size_t
#include <exception>  // for terminate
#include <iostream>   // for cerr, endl
#include <string>     // for string
#include <utility>    // for move

#include "exception.hpp"
#include "exception_stack.hpp"
#include "object_manager.hpp"

namespace except::detail {

void Throw(Type type, std::string file, size_t line) {
    if (stack.empty()) {
        std::cerr << "uncaught exception of type " << Message(type)
                  << " at line " << line << " of file " << file << std::endl;
        std::terminate();
    }

    Node* current = stack.top();
    stack.pop();

    current->exception.type = type;
    current->exception.file = std::move(file);
    current->exception.line = line;

    ObjectManager::UnwindToCheckpoint();

    // NOLINTNEXTLINE(cert-err52-cpp)
    std::longjmp(static_cast<int*>(current->buf),
                 static_cast<int>(Status::RAISED));
}

}  // namespace except::detail
