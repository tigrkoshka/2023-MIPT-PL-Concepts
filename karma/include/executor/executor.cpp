#include "executor.hpp"

#include <cstdint>  // for uint32_t
#include <memory>   // for make_unique
#include <string>   // for string
#include <utility>  // for move

#include "executor/config.hpp"
#include "executor/impl.hpp"

namespace karma {

Executor::Executor()
    : impl_(std::make_unique<Impl>()) {}

Executor::Executor(Config config)
    : impl_(std::make_unique<Impl>(std::move(config))) {}

Executor::~Executor() = default;

Executor::ReturnCode Executor::MustExecute(const std::string& exec) {
    return MustExecute(exec, Config());
}

Executor::ReturnCode Executor::Execute(const std::string& exec) {
    return Execute(exec, Config());
}

Executor::ReturnCode Executor::MustExecute(const std::string& exec,
                                           const Config& config) {
    return impl_->MustExecute(exec, config);
}

Executor::ReturnCode Executor::Execute(const std::string& exec,
                                       const Config& config) {
    return impl_->Execute(exec, config);
}

}  // namespace karma