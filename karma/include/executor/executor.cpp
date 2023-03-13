#include "executor.hpp"

#include <cstdint>   // for uint32_t
#include <memory>    // for make_unique
#include <string>    // for string

#include "impl.hpp"

namespace karma {

Executor::Executor()
    : impl_(std::make_unique<executor::detail::Impl>()) {}

Executor::~Executor() = default;

uint32_t Executor::MustExecute(const std::string& exec_path) {
    return impl_->MustExecute(exec_path);
}

uint32_t Executor::Execute(const std::string& exec_path) {
    return impl_->Execute(exec_path);
}

}  // namespace karma