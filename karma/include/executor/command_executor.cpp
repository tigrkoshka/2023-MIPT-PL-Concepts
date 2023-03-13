#include "command_executor.hpp"

#include "specs/architecture.hpp"

namespace karma::executor::detail {

namespace arch = karma::detail::specs::arch;

arch::Word& CommandExecutor::Reg(arch::Register reg) {
    return storage_->Reg(reg);
}

arch::Word& CommandExecutor::Mem(arch::Address address) {
    return storage_->Mem(address);
}

arch::Word& CommandExecutor::Flags() {
    return storage_->Flags();
}

}  // namespace karma::executor::detail
