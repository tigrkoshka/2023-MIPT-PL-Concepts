#include "throw.hpp"

#include <csetjmp>    // for longjmp
#include <exception>  // for terminate
#include <iostream>   // for cerr, endl
#include <optional>   // for optional

#include "exception_stack.hpp"
#include "object_manager.hpp"

void Throw(except::Type type, std::source_location source_location) {
    std::optional<except::detail::Node*> current_opt =
        except::detail::TryGetCurrent();

    if (!current_opt) {
        std::cerr << "uncaught exception " << Message(type) << " at line "
                  << source_location.line() << " of file "
                  << source_location.file_name() << std::endl;
        std::terminate();
    }

    except::detail::ObjectManager::UnwindToCheckpoint();
    current_opt.value()->Raise(type, source_location);
    std::longjmp(current_opt.value()->Buff(), 1);  // NOLINT(cert-err52-cpp)
}
