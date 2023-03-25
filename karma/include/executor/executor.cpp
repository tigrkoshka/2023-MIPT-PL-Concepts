#include "executor.hpp"

#include <memory>   // for make_unique
#include <string>   // for string
#include <utility>  // for move

#include "executor/config.hpp"
#include "executor/impl.hpp"
#include "utils/logger.hpp"

namespace karma {

Executor::Executor()
    : impl_(std::make_unique<Impl>(Config())) {}

Executor::Executor(Config config)
    : impl_(std::make_unique<Impl>(std::move(config))) {}

Executor::~Executor() = default;

Executor::ReturnCode Executor::MustExecute(const std::string& exec_path,
                                           const Config& config,
                                           Logger log) {
    return impl_->MustExecute(exec_path, config, log.log);
}

Executor::ReturnCode Executor::Execute(const std::string& exec_path,
                                       const Config& config,
                                       Logger log) {
    return impl_->Execute(exec_path, config, log.log);
}

Executor::ReturnCode Executor::MustExecute(const std::string& exec_path,
                                           Logger log) {
    return MustExecute(exec_path, Config(), log);
}

Executor::ReturnCode Executor::Execute(const std::string& exec_path,
                                       Logger log) {
    return Execute(exec_path, Config(), log);
}

}  // namespace karma
