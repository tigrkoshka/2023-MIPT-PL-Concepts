#include "command_executor.hpp"

#include "executor/storage.hpp"
#include "specs/architecture.hpp"

namespace karma {

namespace arch = detail::specs::arch;

void Executor::CommandExecutor::CheckPushAllowed() {
    storage_->CheckPushAllowed();
}

arch::Word& Executor::CommandExecutor::Reg(arch::Register reg,
                                           bool internal_usage) {
    return storage_->Reg(reg, internal_usage);
}

arch::Word& Executor::CommandExecutor::Mem(arch::Address address) {
    return storage_->Mem(address);
}

arch::Word& Executor::CommandExecutor::Flags() {
    return storage_->Flags();
}

}  // namespace karma
