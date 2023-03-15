#pragma once

#include <functional>     // for function
#include <unordered_map>  // for unordered_map

#include "executor/common_executor.hpp"
#include "executor/executor.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"

namespace karma {

class Executor::RIExecutor : public CommonExecutor {
   private:
    using ExecutionError = errors::executor::ExecutionError::Builder;

    using Args      = karma::detail::specs::cmd::args::RIArgs;
    using Operation = std::function<MaybeReturnCode(Args)>;

   private:
    karma::detail::specs::arch::Word ImmWord(const Args&);

   private:
    Operation HALT();
    Operation SYSCALL();
    Operation ADDI();
    Operation SUBI();
    Operation MULI();
    Operation DIVI();
    Operation NOT();
    Operation SHLI();
    Operation SHRI();
    Operation ANDI();
    Operation ORI();
    Operation XORI();
    Operation CMPI();
    Operation PUSH();
    Operation POP();
    Operation LC();

   public:
    using Map = std::unordered_map<karma::detail::specs::cmd::Code, Operation>;
    Map GetMap();
};

}  // namespace karma