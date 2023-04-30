#include "throw.hpp"

#include <csetjmp>    // for longjmp
#include <cstddef>    // for size_t
#include <exception>  // for terminate
#include <iostream>   // for cerr, endl
#include <optional>   // for optional

#include "exception.hpp"
#include "exception_stack.hpp"
#include "object_manager.hpp"

namespace except::detail {

void Throw(Type type, const char* file, size_t line) {
    std::optional<Node*> current_opt = Node::TryGetCurrent();
    if (!current_opt) {
        std::cerr << "uncaught exception " << Message(type) << " at line "
                  << line << " of file " << file << std::endl;
        std::terminate();
    }

    ObjectManager::UnwindToCheckpoint();
    current_opt.value()->Raise(type, file, line);
    std::longjmp(current_opt.value()->Buff(), 1); // NOLINT(cert-err52-cpp)
}

}  // namespace except::detail
