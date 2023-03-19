#include "executor_base.hpp"

#include "executor/storage.hpp"
#include "specs/architecture.hpp"

namespace karma {

namespace arch = detail::specs::arch;

void Executor::ExecutorBase::CheckPushAllowed() {
    return storage_->CheckPushAllowed();
}

arch::Word Executor::ExecutorBase::RReg(arch::Register reg,
                                        bool internal_usage) const {
    return storage_->RReg(reg, internal_usage);
}

arch::Word Executor::ExecutorBase::RMem(arch::Address address) const {
    return storage_->RMem(address);
}

arch::Word& Executor::ExecutorBase::WReg(arch::Register reg,
                                         bool internal_usage) {
    return storage_->WReg(reg, internal_usage);
}

arch::Word& Executor::ExecutorBase::WMem(arch::Address address) {
    return storage_->WMem(address);
}

arch::Word& Executor::ExecutorBase::Flags() {
    return storage_->Flags();
}

}  // namespace karma
