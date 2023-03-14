#include "executor.hpp"

#include <cstdint>  // for uint32_t
#include <memory>   // for make_unique
#include <string>   // for string

#include "impl.hpp"

namespace karma {

Executor::Executor(Config config)
    : impl_(std::make_unique<executor::detail::Impl>(std::move(config))) {}

Executor::~Executor() = default;

uint32_t Executor::MustExecute(const std::string& exec, const Config& config) {
    return impl_->MustExecute(exec, config);
}

uint32_t Executor::Execute(const std::string& exec, const Config& config) {
    return impl_->Execute(exec, config);
}

}  // namespace karma