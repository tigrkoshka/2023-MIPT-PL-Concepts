#include "j_executor.hpp"

#include "executor/executor.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/flags.hpp"

namespace karma {

namespace arch  = detail::specs::arch;
namespace cmd   = detail::specs::cmd;
namespace flags = detail::specs::flags;

Executor::JExecutor::Operation Executor::JExecutor::JMP() {
    return [this](Args args) -> MaybeReturnCode {
        WReg(arch::kInstructionRegister, kInternalUse) = args.addr;
        return {};
    };
}

Executor::JExecutor::Operation Executor::JExecutor::JNE() {
    return [this](Args args) -> MaybeReturnCode {
        Jump(flags::NOT_EQUAL, args.addr);
        return {};
    };
}

Executor::JExecutor::Operation Executor::JExecutor::JEQ() {
    return [this](Args args) -> MaybeReturnCode {
        Jump(flags::EQUAL, args.addr);
        return {};
    };
}

Executor::JExecutor::Operation Executor::JExecutor::JLE() {
    return [this](Args args) -> MaybeReturnCode {
        Jump(flags::LESS_OR_EQUAL, args.addr);
        return {};
    };
}

Executor::JExecutor::Operation Executor::JExecutor::JL() {
    return [this](Args args) -> MaybeReturnCode {
        Jump(flags::LESS, args.addr);
        return {};
    };
}

Executor::JExecutor::Operation Executor::JExecutor::JGE() {
    return [this](Args args) -> MaybeReturnCode {
        Jump(flags::GREATER_OR_EQUAL, args.addr);
        return {};
    };
}

Executor::JExecutor::Operation Executor::JExecutor::JG() {
    return [this](Args args) -> MaybeReturnCode {
        Jump(flags::GREATER, args.addr);
        return {};
    };
}

Executor::JExecutor::Operation Executor::JExecutor::PRC() {
    return [this](Args) -> MaybeReturnCode {
        PrepareCall();
        return {};
    };
}

Executor::JExecutor::Operation Executor::JExecutor::CALLI() {
    return [this](Args args) -> MaybeReturnCode {
        Call(args.addr);
        return {};
    };
}

Executor::JExecutor::Operation Executor::JExecutor::RET() {
    return [this](Args) -> MaybeReturnCode {
        Return();
        return {};
    };
}

Executor::JExecutor::Map Executor::JExecutor::GetMap() {
    return {
        {cmd::JMP,   JMP()  },
        {cmd::JNE,   JNE()  },
        {cmd::JEQ,   JEQ()  },
        {cmd::JLE,   JLE()  },
        {cmd::JL,    JL()   },
        {cmd::JGE,   JGE()  },
        {cmd::JG,    JG()   },
        {cmd::PRC,   PRC()  },
        {cmd::CALLI, CALLI()},
        {cmd::RET,   RET()  },
    };
}

}  // namespace karma
