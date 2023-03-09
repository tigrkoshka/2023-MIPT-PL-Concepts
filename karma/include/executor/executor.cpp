#include "executor.hpp"

#include <memory>  // for make_shared
#include <string>  // for string

#include "impl.hpp"

namespace karma {

Executor::Executor()
    : impl_(std::make_unique<executor::detail::Impl>()) {}

Executor::~Executor() = default;

void Executor::MustExecute(const std::string& exec_path) {
    impl_->MustExecute(exec_path);
}

void Executor::Execute(const std::string& exec_path) {
    impl_->Execute(exec_path);
}

}  // namespace karma