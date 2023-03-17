#include "command_executor.hpp"

#include "executor/storage.hpp"
#include "specs/architecture.hpp"

namespace karma {

namespace arch = detail::specs::arch;

void Executor::CommandExecutor::CheckPushAllowed() {
    return storage_->CheckPushAllowed();
}

arch::Word Executor::CommandExecutor::RReg(arch::Register reg,
                                           bool internal_usage) const {
    return storage_->RReg(reg, internal_usage);
}

arch::Word Executor::CommandExecutor::RMem(arch::Address address) const {
    return storage_->RMem(address);
}

arch::Word& Executor::CommandExecutor::WReg(arch::Register reg,
                                            bool internal_usage) {
    return storage_->WReg(reg, internal_usage);
}

arch::Word& Executor::CommandExecutor::WMem(arch::Address address) {
    return storage_->WMem(address);
}

arch::Word& Executor::CommandExecutor::Flags() {
    return storage_->Flags();
}

}  // namespace karma
